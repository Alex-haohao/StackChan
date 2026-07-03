/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

/// API endpoint configuration for the StackChan backend server
///
/// This class contains all the base URL configurations and API endpoint paths
/// for communicating with the StackChan backend server.
///
/// Backend Configuration:
/// - Pass STACKCHAN_SERVER_HOST at build time instead of hardcoding production
///   infrastructure into source control.
/// - Pass STACKCHAN_SERVER_TLS=true when the server is exposed over HTTPS/WSS.
/// - The base URL is constructed as: `scheme://server/stackChan/`
/// - WebSocket endpoint uses: `ws-scheme://server/stackChan/ws`
class Urls {
  /// Backend server base address configuration
  ///
  /// Format: "server-ip:port", "api.example.com", or "https://api.example.com"
  /// Example:
  /// flutter run --dart-define=STACKCHAN_SERVER_HOST=api.example.com
  /// flutter run --dart-define=STACKCHAN_SERVER_HOST=192.168.1.100:12800
  ///
  /// Keep the default as the upstream placeholder so local builds fail closed
  /// unless a developer intentionally points the app at a backend.
  static const String url = String.fromEnvironment(
    "STACKCHAN_SERVER_HOST",
    defaultValue: "00.000.000.000:0000",
  );

  static const bool _useTls = bool.fromEnvironment(
    "STACKCHAN_SERVER_TLS",
    defaultValue: false,
  );

  static const String _pathPrefix = String.fromEnvironment(
    "STACKCHAN_SERVER_PATH_PREFIX",
    defaultValue: "",
  );

  static const String _xiaozhiApiBaseUrl = String.fromEnvironment(
    "XIAOZHI_API_BASE_URL",
    defaultValue: "",
  );

  static String _normalizeHost(String host) {
    final value = host.trim();
    if (value.startsWith("http://")) {
      return value.substring("http://".length).replaceAll(RegExp(r"/+$"), "");
    }
    if (value.startsWith("https://")) {
      return value.substring("https://".length).replaceAll(RegExp(r"/+$"), "");
    }
    return value.replaceAll(RegExp(r"/+$"), "");
  }

  static String _normalizePathPrefix(String pathPrefix) {
    final value = pathPrefix.trim().replaceAll(RegExp(r"^/+|/+$"), "");
    return value.isEmpty ? "" : "$value/";
  }

  static String _ensureTrailingSlash(String value) {
    final trimmed = value.trim().replaceAll(RegExp(r"/+$"), "");
    return trimmed.isEmpty ? "" : "$trimmed/";
  }

  static String _origin({required String host, required bool useTls}) {
    final scheme = useTls || host.trim().startsWith("https://")
        ? "https"
        : "http";
    return "$scheme://${_normalizeHost(host)}/";
  }

  static String _webSocketOrigin({required String host, required bool useTls}) {
    final scheme = useTls || host.trim().startsWith("https://") ? "wss" : "ws";
    return "$scheme://${_normalizeHost(host)}/";
  }

  static String buildBaseUrl({
    required String host,
    bool useTls = false,
    String pathPrefix = "",
  }) {
    return "${_origin(host: host, useTls: useTls)}"
        "${_normalizePathPrefix(pathPrefix)}stackChan/";
  }

  static String buildFileUrl({
    required String host,
    bool useTls = false,
    String pathPrefix = "",
  }) {
    return "${_origin(host: host, useTls: useTls)}"
        "${_normalizePathPrefix(pathPrefix)}";
  }

  static String buildWebSocketUrl({
    required String host,
    bool useTls = false,
    String pathPrefix = "",
  }) {
    return "${_webSocketOrigin(host: host, useTls: useTls)}"
        "${_normalizePathPrefix(pathPrefix)}stackChan/ws";
  }

  /// Get the HTTP base URL for API requests
  ///
  /// Returns: `http://server-address/stackChan/`
  static String getBaseUrl() {
    return buildBaseUrl(host: url, useTls: _useTls, pathPrefix: _pathPrefix);
  }

  /// Get the HTTP base URL for file operations (uploads and downloads)
  ///
  /// Returns: `http://server-address/`
  static String getFileUrl() {
    return buildFileUrl(host: url, useTls: _useTls, pathPrefix: _pathPrefix);
  }

  /// Get the WebSocket URL for real-time communication
  ///
  /// Returns: `ws://server-address/stackChan/ws`
  static String getWebSocketUrl() {
    return buildWebSocketUrl(
      host: url,
      useTls: _useTls,
      pathPrefix: _pathPrefix,
    );
  }

  static String buildXiaoZhiApiBaseUrl({String configuredBaseUrl = ""}) {
    final normalized = _ensureTrailingSlash(configuredBaseUrl);
    return normalized.isEmpty ? "https://XiaoZhi.me/" : normalized;
  }

  static String getXiaoZhiApiBaseUrl() {
    return buildXiaoZhiApiBaseUrl(configuredBaseUrl: _xiaozhiApiBaseUrl);
  }

  static bool hasCustomXiaoZhiApiBaseUrl() {
    return _ensureTrailingSlash(_xiaozhiApiBaseUrl).isNotEmpty;
  }

  // ===========================================================================
  // Device Management Endpoints
  // ===========================================================================

  /// Device registration endpoint
  /// Register a new device using its MAC address
  static const String registerMac = "api/v2/device/registerMac";

  /// Device information endpoint
  /// Retrieve device details and status
  static const String deviceInfo = "device/info";

  // ===========================================================================
  // Dance Choreography Endpoints
  // ===========================================================================

  /// Dance data endpoint (v1 legacy)
  static const String dance = "dance";

  /// Dance data endpoint (v2)
  /// Create, retrieve, update, and delete dance choreographies
  static const String v2dance = "v2/dance";

  // ===========================================================================
  // Social & Content Endpoints
  // ===========================================================================

  /// Get random list of devices for discovery
  static const String deviceRandomList = "device/randomList";

  /// File upload endpoint for media (images, videos, dance files)
  static const String uploadFile = "uploadFile";

  /// Create a new social post
  static const String postAdd = "post/add";

  /// Retrieve post details
  static const String postGet = "post/get";

  /// Delete a social post
  static const String postDelete = "post/delete";

  /// Create a comment on a post
  static const String postCommentCreate = "post/comment/create";

  /// Delete a comment from a post
  static const String postCommentDelete = "post/comment/delete";

  /// Retrieve comments for a post
  static const String postCommentGet = "post/comment/get";

  /// Panoramic image or 360 view content endpoint
  static const String pano = "pano";

  // ===========================================================================
  // Authentication & User Endpoints
  // ===========================================================================

  /// User login endpoint
  static const String login = "v2/user/login";

  /// User profile management endpoint
  static const String user = "v2/user";

  /// User devices management endpoint
  static const String devices = "v2/devices";

  /// User registration endpoint
  static const String registration = "v2/user/registration";

  // ===========================================================================
  // Device-User Binding Endpoints
  // ===========================================================================

  /// Bind a device to a user account
  static const String v2deviceBind = "v2/device/bind";

  /// Unbind a device from a user account
  static const String v2deviceUnbind = "v2/device/unbind";

  /// Restore device agent configuration
  static const String deviceAgentRestore = "v2/device/agent/restore";

  // ===========================================================================
  // XiaoZhi AI Service Endpoints
  // ===========================================================================

  /// XiaoZhi AI authentication token endpoint
  /// Retrieve token for AI service access
  static const String xiaozhiToken = "xiaozhi/token";

  /// XiaoZhi AI token refresh endpoint
  /// Refresh expired authentication tokens
  static const String xiaozhiTokenRefresh = "xiaozhi/token/refresh";

  /// Generate license token for device activation
  /// Used for StackChan device licensing and activation
  static const String xiaozhiGenerateLicenseToken =
      "xiaozhi/generateLicenseToken";
}
