#ifndef __INCLUDE_STATE_RENDERSTATE_H__
#define __INCLUDE_STATE_RENDERSTATE_H__

#include <Render/RenderNode.h>
#include <State/Word.h>
#include <Util/SmallVector.h>

namespace tex {

const unsigned RENDER_STACK_SIZE = 64;

enum {
  NO_MODE = 0,
  VMODE,
  HMODE,
  MMODE,
  IN_VMODE,
  IN_HMODE,
  IN_MMODE,
};

class RenderState {
private:
  RenderState(const RenderState &);
  RenderState &operator=(const RenderState &);

private:
  struct frame {
    RenderNode *head;
    RenderNode *tail;
    uint32_t prev_graf;
    word aux;
  };

  SmallVector<frame, RENDER_STACK_SIZE> stack;
  frame cur_frame;
  uint8_t r_mode;

private:
public:
  RenderState(void) : stack(), r_mode(NO_MODE) {}

  RenderNode *head(void) {
    return cur_frame.head;
  }

  void set_head(RenderNode *node) {
    cur_frame.head = node;
  }

  RenderNode *tail(void) {
    return cur_frame.tail;
  }

  void set_tail(RenderNode *node) {
    cur_frame.tail = node;
  }

  uint32_t prev_graf(void) {
    return cur_frame.prev_graf;
  }

  void set_prev_graf(uint32_t prev_graf) {
    cur_frame.prev_graf = prev_graf;
  }

  word aux(void) {
    return cur_frame.aux;
  }

  void set_aux(word aux) {
    cur_frame.aux = aux;
  }

  void push(void) {
    stack.push(cur_frame);
  }

  void pop(void) {
    cur_frame = stack.pop();
  }

  uint8_t mode(void) {
    return r_mode;
  }

  void set_mode(uint8_t mode) {
    r_mode = mode;
  }

  void append(RenderNode *node) {
    if (!cur_frame.head) {
      cur_frame.head = cur_frame.tail = node;
    } else {
      cur_frame.tail->link = node;
      cur_frame.tail = node;
    }
  }

  bool empty(void) {
    return cur_frame.head == NULL;
  }
};

}

#endif  // __INCLUDE_STATE_RENDERSTATE_H__