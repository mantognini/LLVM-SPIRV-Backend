# -*- Python -*-

import os

# Setup config name.
config.name = 'HWAddressSanitizer' + getattr(config, 'name_suffix', 'default')

# Setup source root.
config.test_source_root = os.path.dirname(__file__)

# Setup default compiler flags used with -fsanitize=memory option.
clang_cflags = [config.target_cflags] + config.debug_info_flags
clang_cxxflags = config.cxx_mode_flags + clang_cflags
clang_hwasan_cflags = ["-fsanitize=hwaddress", "-mllvm", "-hwasan-globals", "-fuse-ld=lld"] + clang_cflags
if config.target_arch == 'x86_64':
  # This does basically the same thing as tagged-globals on aarch64. Because
  # the x86_64 implementation is for testing purposes only there is no
  # equivalent target feature implemented on x86_64.
  clang_hwasan_cflags += ["-mcmodel=large"]
clang_hwasan_cxxflags = config.cxx_mode_flags + clang_hwasan_cflags

def build_invocation(compile_flags):
  return " " + " ".join([config.clang] + compile_flags) + " "

config.substitutions.append( ("%clangxx ", build_invocation(clang_cxxflags)) )
config.substitutions.append( ("%clang_hwasan ", build_invocation(clang_hwasan_cflags)) )
config.substitutions.append( ("%clangxx_hwasan ", build_invocation(clang_hwasan_cxxflags)) )
config.substitutions.append( ("%compiler_rt_libdir", config.compiler_rt_libdir) )

default_hwasan_opts_str = ':'.join(['disable_allocator_tagging=1', 'random_tags=0'] + config.default_sanitizer_opts)
if default_hwasan_opts_str:
  config.environment['HWASAN_OPTIONS'] = default_hwasan_opts_str
  default_hwasan_opts_str += ':'
config.substitutions.append(('%env_hwasan_opts=',
                             'env HWASAN_OPTIONS=' + default_hwasan_opts_str))

# Default test suffixes.
config.suffixes = ['.c', '.cpp']

if config.host_os not in ['Linux', 'Android']:
  config.unsupported = True
