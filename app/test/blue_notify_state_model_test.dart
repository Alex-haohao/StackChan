import 'package:flutter_test/flutter_test.dart';
import 'package:stack_chan/model/blue_model.dart';

void main() {
  test('parses notifyState device data', () {
    final model = BlueNotifyStateModel.fromJson(
      '{"cmd":"notifyState","data":{"type":4,"state":"cipher"}}',
    );

    expect(model?.cmd, 'notifyState');
    expect(model?.data?.type, 4);
    expect(model?.data?.state, 'cipher');
  });

  test('tolerates non-notify frames with scalar data', () {
    final model = BlueNotifyStateModel.fromJson(
      '{"cmd":"handshake","data":"timestamp"}',
    );

    expect(model, isNotNull);
    expect(model?.cmd, 'handshake');
    expect(model?.data, isNull);
  });

  test('rejects malformed notifyState data', () {
    final model = BlueNotifyStateModel.fromJson(
      '{"cmd":"notifyState","data":"timestamp"}',
    );

    expect(model, isNull);
  });
}
