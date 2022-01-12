import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';

// C function signatures
typedef _CVersionFunc = ffi.Pointer<Utf8> Function();
typedef _CProcessImageFunc = ffi.Void Function(
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>,
);
typedef _CStitchImageFunc = ffi.Void Function(
  ffi.Pointer<Utf8>,
  ffi.Int32,
  ffi.Pointer<Utf8>,
);

// Dart function signatures
typedef _VersionFunc = ffi.Pointer<Utf8> Function();
typedef _ProcessImageFunc = void Function(
  ffi.Pointer<Utf8>,
  ffi.Pointer<Utf8>,
);
typedef _StitchImageFunc = void Function(
  ffi.Pointer<Utf8>,
  int,
  ffi.Pointer<Utf8>,
);

// Getting a library that holds needed symbols
ffi.DynamicLibrary _openDynamicLibrary() {
  if (Platform.isAndroid) {
    return ffi.DynamicLibrary.open('libnative_opencv.so');
  } else if (Platform.isWindows) {
    return ffi.DynamicLibrary.open("native_opencv_windows_plugin.dll");
  }

  return ffi.DynamicLibrary.process();
}

ffi.DynamicLibrary _lib = _openDynamicLibrary();

// Looking for the functions
final _VersionFunc _version =
    _lib.lookup<ffi.NativeFunction<_CVersionFunc>>('version').asFunction();
final _ProcessImageFunc _processImage = _lib
    .lookup<ffi.NativeFunction<_CProcessImageFunc>>('process_image')
    .asFunction();
final _StitchImageFunc _stitchImage = _lib
    .lookup<ffi.NativeFunction<_CStitchImageFunc>>('stitch_image')
    .asFunction();

String opencvVersion() {
  return _version().toDartString();
}

void processImage(ProcessImageArguments args) {
  _processImage(args.inputPath.toNativeUtf8(), args.outputPath.toNativeUtf8());
}

void stitchImage(StitchImageArguments args) {
  /*final ffi.Pointer<ffi.Pointer<Utf8>> pointer =
      strListToPointer(args.inputPaths);
  print('Get paths success');

  try {
    _stitchImage(args.inputPaths.toString().toNativeUtf8(), args.size,
        args.outputPath.toNativeUtf8());

    malloc.free(pointer);
    print('_stitchImage success');
  } catch (_) {
    malloc.free(pointer);
    print('_stitchImage failed');
  }*/

  _stitchImage(args.inputPaths.toString().toNativeUtf8(), args.size,
      args.outputPath.toNativeUtf8());
}

/// Don't forget to run malloc.free with result!
ffi.Pointer<ffi.Pointer<Utf8>> strListToPointer(List<String> stringList) {
  final List<ffi.Pointer<Utf8>> utf8PointerList =
      stringList.map((str) => str.toNativeUtf8()).toList();

  final ffi.Pointer<ffi.Pointer<Utf8>> pointerPointer =
      malloc.allocate(stringList.length);

  for (int i = 0; i < stringList.length; i++) {
    pointerPointer[i] = utf8PointerList[i];
  }

  /*strings.asMap().forEach((index, utf) {
    pointerPointer[index] = utf8PointerList[index];
  });*/

  return pointerPointer;
}

class ProcessImageArguments {
  final String inputPath;
  final String outputPath;

  ProcessImageArguments(this.inputPath, this.outputPath);
}

class StitchImageArguments {
  final List<String> inputPaths;
  int size;
  final String outputPath;

  StitchImageArguments(this.inputPaths, this.size, this.outputPath);
}
