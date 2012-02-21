#include <stdlib.h>

#include <Type/TFM.h>


using namespace tex;

char *TFM::create_fix_word_string(int32_t fix_word) {
  char left_str[5] = {'\0'};
  bool sign = false;
  if (fix_word < 0) {
    sign = true;
    fix_word = -fix_word;
  }
  
  uint32_t left = fix_word >> 20;
  uint32_t right = fix_word & ((1 << 20) - 1);
  snprintf(left_str, sizeof(left_str), "%d", left);
  
  // now for the calculation of the decimals.
  // <part to right of decimal> = right / 2^20
  const unsigned right_size = 20; // we never need more than 20 decimal places.
  char right_str[right_size + 1] = "00000000000000000000";
  for (unsigned r_idx = 0; r_idx < right_size; r_idx++) {
    right *= 10;
    unsigned digit = right / (1 << 20);
    right_str[r_idx] = digit + '0';
    right -= (digit * 1<<20);
  }
  
  // now allocate the string.
  unsigned string_size = 1 // sign
                       + 4 // left
                       + 1 // decimal
                       + right_size // right
                       + 1; // nul terminator
  char *str = (char*)malloc(string_size);
  if (sign)
    snprintf(str, string_size, "-%s.%s", left_str, right_str);
  else
    snprintf(str, string_size, "%s.%s", left_str, right_str);
  return str;
}

Diag *TFM::read_header(UniquePtr<BinaryInputStream> &stream, uint16_t lh, TFM *tfm) {
  if (!lh)
    return NULL;
  
  if (stream->read_uint32(tfm->file_checksum))
    assert(false && "Unexpected EOF! Internal state error.");
  
  if (lh == 1)
    return NULL;
  
  if (stream->read_uint32((uint32_t&)tfm->design_size))
    assert(false && "Unexpected EOF! Internal state error.");
  
  if (lh == 2)
    return NULL;
    
  if (lh < 11)
    return new GenericDiag("TFM header is a bad size.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  uint8_t char_code_size;
  if (stream->read_uint8(char_code_size))
    assert(false && "Unexpected EOF! Internal state error.");
  
  if (char_code_size > 39)
    return new GenericDiag("TFM header has a bad character coding convention string length byte.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  if (stream->read_bytes((uint8_t*)tfm->char_code_conv, 39))
    assert(false && "Unexpected EOF! Internal state error.");
  
  tfm->char_code_conv[char_code_size] = '\0';
  
  if (lh == 11)
    return NULL;
  
  if (lh < 16)
    return new GenericDiag("TFM header is a bad size.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  uint8_t font_id_size;
  if (stream->read_uint8(font_id_size))
    assert(false && "Unexpected EOF! Internal state error.");
  
  if (font_id_size > 19)
    return new GenericDiag("TFM header has bad font identiifcation string length byte.", DIAG_TFM_PARSE_ERR, BLAME_HERE);

  if (stream->read_bytes((uint8_t*)tfm->font_id, 19))
    assert(false && "Unexpected EOF! Internal state error.");
  
  tfm->font_id[font_id_size] = '\0';
  
  if (lh == 16)
    return NULL;
  
  uint32_t seven_face;
  if (stream->read_uint32(seven_face))
    assert(false && "Unexpected EOF! Internal state error.");
  
  tfm->seven_bit_safe_flag = seven_face & 0xFF;
  tfm->font_face = seven_face >> 24;
  
  if (lh > 17) {
    stream->seek(stream->offset() + (lh - 17) * 4);
  }
  
  return NULL;
}

void TFM::read_char_info(UniquePtr<BinaryInputStream> &stream, TFM *tfm) {
  unsigned chars = tfm->char_upper - tfm->char_lower + 1;
  for (unsigned c = 0; c < chars; c++) {
    // read a single char_info
    char_info_word info;
    if (stream->read_uint8(info.width_index))
      assert(false && "Unexpected EOF! Internal state error.");
    uint8_t height_depth;
    if (stream->read_uint8(height_depth))
      assert(false && "Unexpected EOF! Internal state error.");
    info.height_index = (height_depth >> 4);
    info.depth_index = height_depth & 0x0F;
    
    uint8_t italic_tag;
    if (stream->read_uint8(italic_tag))
      assert(false && "Unexpected EOF! Internal state error.");
    
    info.italic_index = italic_tag >> 2;
    info.tag = italic_tag & 0x3;
    
    if (stream->read_uint8(info.remainder))
      assert(false && "Unexpected EOF! Internal state error.");
    
    tfm->char_info[c] = info;
  }
}

static void read_fix_word_table(UniquePtr<BinaryInputStream> &stream, TFM::fix_word *table, uint16_t size) {
  for (uint16_t f = 0; f < size; f++) {
    uint32_t word;
    if (stream->read_uint32(word))
      assert(false && "Unexpected EOF! Internal state error.");
    table[f] = (TFM::fix_word)word;
  }
}


Diag *TFM::init_from_file(const char *path, UniquePtr<TFM> &result) {
  UniquePtr<BinaryInputStream> stream;
  Diag *diag;
  if ((diag = BinaryInputStream::init_from_file(path, stream)))
    return diag;
  
  // TFM files are big endian
  stream->set_endian(ENDIAN_BIG);
  
  // The first 24 bytes are guaranteed.
  if (stream->size() < 24)
    return new GenericDiag("TFM file not large enough.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  uint16_t lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np;
  if (stream->read_uint16(lf))
    return new GenericDiag("TFM file empty.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  if ((lf < 6))
    return new GenericDiag("TFM file not large enough to hold header.", DIAG_TFM_PARSE_ERR, BLAME_HERE);

  if ((lf * 4) != stream->size())
    return new GenericDiag("TFM file incorrect size.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  if    (stream->read_uint16(lh)
      || stream->read_uint16(bc)
      || stream->read_uint16(ec)
      || stream->read_uint16(nw)
      || stream->read_uint16(nh)
      || stream->read_uint16(nd)
      || stream->read_uint16(ni)
      || stream->read_uint16(nl)
      || stream->read_uint16(nk)
      || stream->read_uint16(ne)
      || stream->read_uint16(np)) {
        // The size of the binary stream is at least 6 words, or 24 bytes, so reading these
        // halfwords should not be an issue. If there is, someone lied somewhere about their
        // size.
        assert(false && "Expected to read header without EOF occurring; internal error.");
      }

  if ((bc > 0 && (bc - 1) > ec) || ec > 255)
    return new GenericDiag("Bad character range in TFM file.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  if (ne > 256)
    return new GenericDiag("Extensible character table larger than valid size.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  uint16_t check_size = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np;
  if (check_size != lf)
    return new GenericDiag("Size of file does not equal expected size of file.", DIAG_TFM_PARSE_ERR, BLAME_HERE);
  
  // Ok, so now we're ready to actually commit to an object.
  TFM *tfm = new TFM();

  tfm->width_size = nw;
  tfm->height_size = nh;
  tfm->depth_size = nd;
  tfm->italic_size = ni;
  
  tfm->char_lower = bc;
  tfm->char_upper = ec;
  
  tfm->char_code_conv[0] = '\0'; // empty character encoding convention by default.
  tfm->font_id[0] = '\0';

  tfm->width_table = new fix_word[tfm->width_size];
  tfm->height_table = new fix_word[tfm->height_size];
  tfm->depth_table = new fix_word[tfm->depth_size];
  tfm->italic_table = new fix_word[tfm->italic_size];
  
  tfm->char_info = new char_info_word[tfm->char_upper - tfm->char_lower + 1];
  
  // let's read the remainder of the TFM file.
  if ((diag = read_header(stream, lh, tfm))) {
    delete tfm;
    return diag;
  }
  
  read_char_info(stream, tfm);
  read_fix_word_table(stream, tfm->width_table, tfm->width_size);
  read_fix_word_table(stream, tfm->height_table, tfm->height_size);
  read_fix_word_table(stream, tfm->depth_table, tfm->depth_size);
  read_fix_word_table(stream, tfm->italic_table, tfm->italic_size);
  
  result.reset(tfm);
  
  return NULL;
}

TFM::~TFM(void) {
  delete[] width_table;
  delete[] height_table;
  delete[] depth_table;
  delete[] italic_table;
  delete[] char_info;
}