# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'conditions': [
    ['OS=="android"', {
      'targets': [
        {
          'target_name': 'protobuf_nano_javalib',
          'type' : 'none',
          'variables': {
            # Using empty dir and additionalk_src_dirs since the nano package
            # does not have a src/ subfolder.
            'java_in_dir': '../../build/android/empty',
            'additional_src_dirs': [ 'src/java/src/main/java/com/google/protobuf/nano' ],
          },
          'includes': [ '../../build/java.gypi' ],
        },
        {
          # This proto compiler supports the nano profile, but should only be used for Android.
          'target_name': 'android_protoc',
          'type': 'executable',
          'variables': {
            'chromium_code': 0,
          },
          'toolsets': [ 'host' ],
          'sources': [
            'src/src/google/protobuf/descriptor.cc',
            'src/src/google/protobuf/descriptor.pb.cc',
            'src/src/google/protobuf/descriptor_database.cc',
            'src/src/google/protobuf/dynamic_message.cc',
            'src/src/google/protobuf/extension_set.cc',
            'src/src/google/protobuf/extension_set_heavy.cc',
            'src/src/google/protobuf/generated_message_reflection.cc',
            'src/src/google/protobuf/generated_message_util.cc',
            'src/src/google/protobuf/message.cc',
            'src/src/google/protobuf/message_lite.cc',
            'src/src/google/protobuf/reflection_ops.cc',
            'src/src/google/protobuf/repeated_field.cc',
            'src/src/google/protobuf/service.cc',
            'src/src/google/protobuf/text_format.cc',
            'src/src/google/protobuf/unknown_field_set.cc',
            'src/src/google/protobuf/wire_format.cc',
            'src/src/google/protobuf/wire_format_lite.cc',
            'src/src/google/protobuf/compiler/code_generator.cc',
            'src/src/google/protobuf/compiler/command_line_interface.cc',
            'src/src/google/protobuf/compiler/importer.cc',
            'src/src/google/protobuf/compiler/main.cc',
            'src/src/google/protobuf/compiler/parser.cc',
            'src/src/google/protobuf/compiler/plugin.cc',
            'src/src/google/protobuf/compiler/plugin.pb.cc',
            'src/src/google/protobuf/compiler/subprocess.cc',
            'src/src/google/protobuf/compiler/zip_writer.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_enum.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_enum_field.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_extension.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_field.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_file.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_generator.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_helpers.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_message.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_message_field.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_primitive_field.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_service.cc',
            'src/src/google/protobuf/compiler/cpp/cpp_string_field.cc',
            'src/src/google/protobuf/compiler/java/java_enum.cc',
            'src/src/google/protobuf/compiler/java/java_enum_field.cc',
            'src/src/google/protobuf/compiler/java/java_extension.cc',
            'src/src/google/protobuf/compiler/java/java_field.cc',
            'src/src/google/protobuf/compiler/java/java_file.cc',
            'src/src/google/protobuf/compiler/java/java_generator.cc',
            'src/src/google/protobuf/compiler/java/java_helpers.cc',
            'src/src/google/protobuf/compiler/java/java_message.cc',
            'src/src/google/protobuf/compiler/java/java_message_field.cc',
            'src/src/google/protobuf/compiler/java/java_primitive_field.cc',
            'src/src/google/protobuf/compiler/java/java_service.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_enum.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_enum_field.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_field.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_file.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_generator.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_helpers.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_message.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_message_field.cc',
            'src/src/google/protobuf/compiler/javamicro/javamicro_primitive_field.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_enum.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_enum_field.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_extension.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_field.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_file.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_generator.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_helpers.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_message.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_message_field.cc',
            'src/src/google/protobuf/compiler/javanano/javanano_primitive_field.cc',
            'src/src/google/protobuf/compiler/python/python_generator.cc',
            'src/src/google/protobuf/io/coded_stream.cc',
            'src/src/google/protobuf/io/gzip_stream.cc',
            'src/src/google/protobuf/io/printer.cc',
            'src/src/google/protobuf/io/tokenizer.cc',
            'src/src/google/protobuf/io/zero_copy_stream.cc',
            'src/src/google/protobuf/io/zero_copy_stream_impl.cc',
            'src/src/google/protobuf/io/zero_copy_stream_impl_lite.cc',
            'src/src/google/protobuf/stubs/common.cc',
            'src/src/google/protobuf/stubs/hash.cc',
            'src/src/google/protobuf/stubs/once.cc',
            'src/src/google/protobuf/stubs/structurally_valid.cc',
            'src/src/google/protobuf/stubs/strutil.cc',
            'src/src/google/protobuf/stubs/substitute.cc',
          ],
          'include_dirs': [
            'src/android',
            'src/src',
          ],
          'conditions': [
            ['clang==1', {
              'cflags': [
                '-Wno-null-conversion',
                '-Wno-tautological-undefined-compare',
              ],
            }],
          ],
          'defines': [
            # This macro must be defined to suppress the use
            # of dynamic_cast<>, which requires RTTI.
            'GOOGLE_PROTOBUF_NO_RTTI',
            'GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER',
          ],
          'dependencies': [
            '../zlib/zlib.gyp:zlib',
          ],
        },
      ],
    }],
  ],
}