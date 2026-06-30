import 'dart:convert';

import 'package:flutter_test/flutter_test.dart';
import 'package:stack_chan/util/ble_json_frame_decoder.dart';

void main() {
  test('passes through a complete JSON frame', () {
    final decoder = BleJsonFrameDecoder();
    final frames = decoder.add(
      utf8.encode('{"cmd":"notifyState","data":{"type":3,"state":"ok"}}'),
    );

    expect(frames.map(utf8.decode), [
      '{"cmd":"notifyState","data":{"type":3,"state":"ok"}}',
    ]);
  });

  test('reassembles a JSON frame split across BLE notifications', () {
    final decoder = BleJsonFrameDecoder();

    expect(decoder.add(utf8.encode('{"cmd":"notify')), isEmpty);
    expect(decoder.add(utf8.encode('State","data":{"type":4,')), isEmpty);

    final frames = decoder.add(utf8.encode('"state":"cipher"}}'));

    expect(frames.map(utf8.decode), [
      '{"cmd":"notifyState","data":{"type":4,"state":"cipher"}}',
    ]);
  });

  test('extracts multiple frames from one notification', () {
    final decoder = BleJsonFrameDecoder();
    final frames = decoder.add(
      utf8.encode('{"cmd":"a","data":{}}{"cmd":"b","data":{}}'),
    );

    expect(frames.map(utf8.decode), [
      '{"cmd":"a","data":{}}',
      '{"cmd":"b","data":{}}',
    ]);
  });

  test('reassembles long handshake response-sized JSON', () {
    final decoder = BleJsonFrameDecoder();
    final payload = jsonEncode({
      'cmd': 'notifyState',
      'data': {'type': 4, 'state': List.filled(344, 'A').join()},
    });
    final bytes = utf8.encode(payload);
    final frames = <List<int>>[];

    for (var offset = 0; offset < bytes.length; offset += 64) {
      final end = (offset + 64).clamp(0, bytes.length);
      frames.addAll(decoder.add(bytes.sublist(offset, end)));
    }

    expect(frames.map(utf8.decode), [payload]);
  });
}
