#pragma once

namespace services::web_settings {

/** Initializes the background WebServer and mDNS responder. */
void init();

/** Called repeatedly in loop() to handle incoming HTTP clients. */
void handle();

}  // namespace services::web_settings