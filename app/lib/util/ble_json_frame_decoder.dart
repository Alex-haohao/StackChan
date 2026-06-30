import 'dart:convert';

class BleJsonFrameDecoder {
  BleJsonFrameDecoder({this.maxBufferedBytes = 8192});

  final int maxBufferedBytes;
  final StringBuffer _buffer = StringBuffer();
  int _depth = 0;
  bool _inString = false;
  bool _escape = false;
  bool _started = false;

  List<List<int>> add(List<int> chunk) {
    final frames = <List<int>>[];
    final text = utf8.decode(chunk, allowMalformed: true);

    for (final codePoint in text.runes) {
      final char = String.fromCharCode(codePoint);
      if (!_started) {
        if (char.trim().isEmpty) {
          continue;
        }
        if (char != '{') {
          reset();
          continue;
        }
        _started = true;
      }

      _buffer.write(char);

      if (_inString) {
        if (_escape) {
          _escape = false;
        } else if (char == '\\') {
          _escape = true;
        } else if (char == '"') {
          _inString = false;
        }
      } else if (char == '"') {
        _inString = true;
      } else if (char == '{') {
        _depth++;
      } else if (char == '}') {
        _depth--;
        if (_depth == 0) {
          frames.add(utf8.encode(_buffer.toString()));
          reset();
        } else if (_depth < 0) {
          reset();
        }
      }

      if (_buffer.length > maxBufferedBytes) {
        reset();
      }
    }

    return frames;
  }

  void reset() {
    _buffer.clear();
    _depth = 0;
    _inString = false;
    _escape = false;
    _started = false;
  }
}
