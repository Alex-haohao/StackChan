import 'package:flutter_test/flutter_test.dart';
import 'package:stack_chan/network/urls.dart';
import 'package:stack_chan/util/XiaoZhi_util.dart';

void main() {
  group('Urls XiaoZhi API base URL', () {
    test('defaults to official XiaoZhi API when no override is configured', () {
      expect(
        Urls.buildXiaoZhiApiBaseUrl(configuredBaseUrl: ''),
        'https://XiaoZhi.me/',
      );
    });

    test('normalizes a self-hosted StackChan facade URL override', () {
      expect(
        Urls.buildXiaoZhiApiBaseUrl(
          configuredBaseUrl:
              'https://console.alexhaohao.com/stackChan/xiaozhi/compat',
        ),
        'https://console.alexhaohao.com/stackChan/xiaozhi/compat/',
      );
    });
  });

  group('XiaoZhi MCP access point address', () {
    test('keeps self-hosted full websocket addresses unchanged', () {
      expect(
        XiaoZhiUtil.buildMcpAccessPointAddress(
          'wss://console.alexhaohao.com/mcp/?token=abc',
        ),
        'wss://console.alexhaohao.com/mcp/?token=abc',
      );
    });

    test('keeps official token compatibility for legacy responses', () {
      expect(
        XiaoZhiUtil.buildMcpAccessPointAddress('abc'),
        'wss://api.XiaoZhi.me/mcp/?token=abc',
      );
    });
  });
}
