import 'package:flutter_test/flutter_test.dart';
import 'package:stack_chan/network/urls.dart';
import 'package:stack_chan/util/value_constant.dart';

void main() {
  group('StackChan backend URL configuration', () {
    test('builds default HTTP endpoints from a host and port', () {
      expect(
        Urls.buildBaseUrl(host: '192.0.2.10:12800'),
        'http://192.0.2.10:12800/stackChan/',
      );
      expect(
        Urls.buildFileUrl(host: '192.0.2.10:12800'),
        'http://192.0.2.10:12800/',
      );
      expect(
        Urls.buildWebSocketUrl(host: '192.0.2.10:12800'),
        'ws://192.0.2.10:12800/stackChan/ws',
      );
    });

    test('switches HTTP and WebSocket schemes together for TLS', () {
      expect(
        Urls.buildBaseUrl(host: 'api.example.com', useTls: true),
        'https://api.example.com/stackChan/',
      );
      expect(
        Urls.buildWebSocketUrl(host: 'api.example.com', useTls: true),
        'wss://api.example.com/stackChan/ws',
      );
    });

    test('normalizes schemes, slashes, and reverse-proxy path prefixes', () {
      expect(
        Urls.buildBaseUrl(
          host: 'https://api.example.com/',
          pathPrefix: '/stackchan-prod/',
        ),
        'https://api.example.com/stackchan-prod/stackChan/',
      );
      expect(
        Urls.buildFileUrl(
          host: 'http://api.example.com/',
          pathPrefix: 'stackchan-prod',
        ),
        'http://api.example.com/stackchan-prod/',
      );
    });
  });

  group('StackChan RSA configuration', () {
    test('fails closed when RSA keys are not provided at build time', () {
      expect(ValueConstant.serverPublicKey, isEmpty);
      expect(ValueConstant.clientPrivateKey, isEmpty);
      expect(ValueConstant.stackChanBluePrivateKey, isEmpty);
    });
  });
}
