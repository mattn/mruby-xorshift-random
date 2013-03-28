/*
** random.c - Random module
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/data.h>

#define GLOBAL_RAND_SEED_KEY    "$xor128_seed"

static mrb_value get_opt(mrb_state* mrb)
{
  mrb_value arg;

  arg = mrb_fixnum_value(0);
  mrb_get_args(mrb, "|o", &arg);

  if (!mrb_nil_p(arg)) {
    if (!mrb_fixnum_p(arg)) {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument type");
    }
    arg = mrb_check_convert_type(mrb, arg, MRB_TT_FIXNUM, "Fixnum", "to_int");
    if (mrb_fixnum(arg) < 0) {
      arg = mrb_fixnum_value(0 - mrb_fixnum(arg));
    }
  }
  return arg;
}

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t z;
  uint32_t w;
} xor128_t;

static void xor128_free(mrb_state *mrb, void *ptr)
{
  mrb_free(mrb, ptr);
}

static struct mrb_data_type xor128_type = { "Xor128", xor128_free };

static void xor128_init(xor128_t* s) { 
  s->x = 123456789;
  s->y = 362436069;
  s->z = 521288629;
  s->w = 88675123; 
}

uint32_t xor128(xor128_t* s) { 
  uint32_t t;
 
  t = s->x ^ (s->x << 11);
  s->x = s->y; s->y = s->z; s->z = s->w;
  return s->w = (s->w ^ (s->w >> 19)) ^ (t ^ (t >> 8)); 
}

static mrb_value mrb_rand(mrb_state* mrb, mrb_value self)
{
  xor128_t* s;
  uint32_t r;
  mrb_value max;

  max = get_opt(mrb);
  s = (xor128_t*) DATA_PTR(self);
  r = xor128(s);
  if (mrb_nil_p(max) || mrb_fixnum(max) == 0) {
    return mrb_float_value(r / (double) 0xffffffff);
  }
  return mrb_fixnum_value(r % mrb_fixnum(max));
}

static mrb_value mrb_srand(mrb_state* mrb, mrb_value self)
{
  xor128_t* s;
  uint32_t old;
  mrb_value seed;

  seed = get_opt(mrb);
  if (mrb_nil_p(seed)) {
    seed = mrb_fixnum_value(123456789);
  }
  s = (xor128_t*) DATA_PTR(self);
  old = s->x;
  s->x = mrb_fixnum(seed);
  return mrb_fixnum_value(old);
}

static mrb_value mrb_random_g_srand(mrb_state *mrb, mrb_value self)
{
  return mrb_srand(mrb, mrb_gv_get(mrb, mrb_intern(mrb, "$xor128")));
}

static mrb_value mrb_random_g_rand(mrb_state *mrb, mrb_value self)
{
  return mrb_rand(mrb, mrb_gv_get(mrb, mrb_intern(mrb, "$xor128")));
}

static mrb_value mrb_random_srand(mrb_state *mrb, mrb_value self)
{
  return mrb_srand(mrb, self);
}

static mrb_value mrb_random_rand(mrb_state *mrb, mrb_value self)
{
  return mrb_rand(mrb, self);
}

static mrb_value mrb_random_init(mrb_state *mrb, mrb_value self)
{
  DATA_PTR(self) = mrb_malloc(mrb, sizeof(xor128_t));
  DATA_TYPE(self) = &xor128_type;
  xor128_init((xor128_t*) DATA_PTR(self));
  mrb_random_srand(mrb, self);
  return self;
}

void mrb_mruby_xorshift_random_gem_init(mrb_state *mrb)
{
  struct RClass *random;

  mrb_define_method(mrb, mrb->kernel_module, "rand", mrb_random_g_rand, ARGS_OPT(1));
  mrb_define_method(mrb, mrb->kernel_module, "srand", mrb_random_g_srand, ARGS_OPT(1));

  random = mrb_define_class(mrb, "Xor128Random", mrb->object_class);
  MRB_SET_INSTANCE_TT(random, MRB_TT_DATA);

  mrb_define_class_method(mrb, random, "rand", mrb_random_g_rand, ARGS_OPT(1));
  mrb_define_class_method(mrb, random, "srand", mrb_random_g_srand, ARGS_OPT(1));

  mrb_define_method(mrb, random, "initialize", mrb_random_init, ARGS_OPT(1));
  mrb_define_method(mrb, random, "rand", mrb_random_rand, ARGS_OPT(1));
  mrb_define_method(mrb, random, "srand", mrb_random_srand, ARGS_OPT(1));

  mrb_gv_set(mrb, mrb_intern(mrb, "$xor128"), mrb_class_new_instance(mrb, 0, NULL, random));
}

void mrb_mruby_xorshift_random_gem_final(mrb_state *mrb)
{
}
