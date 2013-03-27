/*
** random.c - Random module
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include "mruby/variable.h"

#define GLOBAL_RAND_SEED_KEY    "$mrb_g_rand_seed"
#define INSTANCE_RAND_SEED_KEY  "$mrb_i_rand_seed"

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

#define NEXT_SEED(x) (mrb_fixnum(x) * 214013 + 2531011)
#define NEXT_RAND(i) ((i < 0 ? i - 0x80000000 : i) / 0x10000 % 0x8000)

static mrb_value mrb_rand(mrb_state* mrb, mrb_value self)
{
  int i;
  mrb_value max;
  mrb_value seed;

  max = get_opt(mrb);
  if (mrb_nil_p(self)) {
    seed = mrb_gv_get(mrb, mrb_intern(mrb, GLOBAL_RAND_SEED_KEY));
    i = NEXT_SEED(seed);
    mrb_gv_set(mrb, mrb_intern(mrb, GLOBAL_RAND_SEED_KEY), mrb_fixnum_value(i));
  } else {
    seed = mrb_iv_get(mrb, self, mrb_intern(mrb, INSTANCE_RAND_SEED_KEY));
    i = NEXT_SEED(seed);
    mrb_iv_set(mrb, self, mrb_intern(mrb, INSTANCE_RAND_SEED_KEY), mrb_fixnum_value(i));
  }
  if (mrb_nil_p(max) || mrb_fixnum(max) == 0) {
    return mrb_float_value(NEXT_RAND(i) / 32767.0);
  }
  return mrb_float_value(NEXT_RAND(i) % mrb_fixnum(max));
}

static mrb_value mrb_srand(mrb_state* mrb, mrb_value self)
{
  mrb_value old_seed;
  mrb_value seed;

  seed = get_opt(mrb);
  if (mrb_nil_p(seed)) {
    seed = mrb_fixnum_value(0); /* TODO */
  }
  if (mrb_nil_p(self)) {
    old_seed = mrb_gv_get(mrb, mrb_intern(mrb, GLOBAL_RAND_SEED_KEY));
    mrb_gv_set(mrb, mrb_intern(mrb, GLOBAL_RAND_SEED_KEY), seed);
  } else {
    old_seed = mrb_iv_get(mrb, self, mrb_intern(mrb, INSTANCE_RAND_SEED_KEY));
    mrb_iv_set(mrb, self, mrb_intern(mrb, INSTANCE_RAND_SEED_KEY), seed);
  }
  return old_seed;
}

static mrb_value mrb_random_g_srand(mrb_state *mrb, mrb_value self)
{
  return mrb_srand(mrb, mrb_nil_value());
}

static mrb_value mrb_random_g_rand(mrb_state *mrb, mrb_value self)
{
  return mrb_rand(mrb, mrb_nil_value());
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
  mrb_random_srand(mrb, self);
  return self;
}

void mrb_mruby_random_vs2010_gem_init(mrb_state *mrb)
{
  struct RClass *random;

  mrb_define_method(mrb, mrb->kernel_module, "rand", mrb_random_g_rand, ARGS_OPT(1));
  mrb_define_method(mrb, mrb->kernel_module, "srand", mrb_random_g_srand, ARGS_OPT(1));

  random = mrb_define_class(mrb, "Random", mrb->object_class);
  mrb_define_class_method(mrb, random, "rand", mrb_random_g_rand, ARGS_OPT(1));
  mrb_define_class_method(mrb, random, "srand", mrb_random_g_srand, ARGS_OPT(1));

  mrb_define_method(mrb, random, "initialize", mrb_random_init, ARGS_OPT(1));
  mrb_define_method(mrb, random, "rand", mrb_random_rand, ARGS_OPT(1));
  mrb_define_method(mrb, random, "srand", mrb_random_srand, ARGS_OPT(1));
}

void mrb_mruby_random_vs2010_gem_final(mrb_state *mrb)
{
}
