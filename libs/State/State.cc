/*****************************************************************************
*  Copyright (c) 2012 Duane Ryan Bailey                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*****************************************************************************/

#include <Config.h>
#include <State/State.h>
#include <Type/TFM.h>
#include <Render/Units.h>

using namespace tex;

State::State(void) {
  // first initialize ccode to all "CC_OTHER_CHAR" as the default code.
  for (unsigned i = 0; i < 128; i++) {
    ccode[i] = CC_OTHER_CHAR;
  }

    // now initialize letters.
  for (unsigned i = 0; i < 26; i++) {
    ccode['A' + i] = CC_LETTER;
    ccode['a' + i] = CC_LETTER;
  }
  
  ccode[0x00] = CC_IGNORE;
  ccode[0x20] = CC_SPACER; // ' ' 
  ccode[0x5C] = CC_ESCAPE; // '\\'
  ccode[0x25] = CC_COMMENT; // '%'
  ccode[0x7F] = CC_INVALID;
  ccode[0x0A] = CC_CAR_RET; // '\n'; technically deviates from tex.
  ccode[0x0D] = CC_CAR_RET; // '\r'

  // load the null font.
  Font null_font;
  fonts.append(null_font);

  // and load computer modern.
  curr_font = load_font("cmr10", "rm", -1000);
  load_font("cmti10", "it", -1000);
  load_font("cmbx10", "bf", -1000);
  load_font("cmtt10", "tt", -1000);

  // Internal variables
  tex_mem[LEFT_SKIP_CODE].scaled = scaled(0);
  tex_mem[RIGHT_SKIP_CODE].scaled = scaled(0);
  tex_mem[HSIZE_CODE].scaled = scaled_from(0x67FFF, UNIT_IN); // 6.5in
  tex_mem[VSIZE_CODE].scaled = scaled_from(9 << 16, UNIT_IN); // 9in
  tex_mem[PARINDENT_CODE].scaled = scaled(18 << 16);
  tex_mem[BASELINE_SKIP_CODE].scaled = scaled(12 << 16);
  tex_mem[SPLIT_TOP_SKIP_CODE].scaled = scaled(12 << 16);

  // enter vmode.
  r_state.set_mode(VMODE);

  // initialize primitives.
  primitive("par", CC_PAR_END, (word){0});
  primitive("end", CC_STOP, (word){0});
}

void State::init(UniquePtr<State> &result) {
    result.reset(new State());
}

uint32_t State::load_font(const char *file, const char *name, int32_t at) {
  uint32_t f = fonts.entries();
  Font empty_font;
  fonts.append(empty_font);
  Font &font = fonts.get(f);
  Path path;
  path.set_area(FONT_AREA);
  path.set_file(file);
  path.set_ext("tfm");
  TFM::load_font(path.full_path().c_str(), font, at);
  primitive(name, CC_SET_FONT, (word){f});
  return f;
}
