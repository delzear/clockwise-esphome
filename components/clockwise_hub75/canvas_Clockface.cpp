#include "canvas_Clockface.h"

namespace canvas {

static unsigned long lastMillis = 0;
static DynamicJsonDocument doc(32768);

Clockface::Clockface(Adafruit_GFX *display, std::string server, std::string file)
    : _display(display), _server(server), _file(file)
{
  Locator::provide(display);
    ESP_LOGI("TAG", "Creating clockface");

}

void Clockface::setup(CWDateTime *dateTime)
{
  this->_dateTime = dateTime;
  drawSplashScreen(0xFFE0, "Downloading...");
  
  if (deserializeDefinition()) {
    clockfaceSetup();
    _is_setup = true;
  } else {
    _is_setup = false;
  }
}

void Clockface::printCenter(const char *msg, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  Locator::getDisplay()->setFont(&Picopixel);
  Locator::getDisplay()->getTextBounds(msg, 0, y, &x1, &y1, &w, &h);
  Locator::getDisplay()->setCursor(32 - (w / 2), y);
  Locator::getDisplay()->setTextColor(0xffff);
  Locator::getDisplay()->print(msg);
}

void Clockface::drawSplashScreen(uint16_t color, const char *msg) {
  Locator::getDisplay()->fillRect(0, 0, 64, 64, 0);
  Locator::getDisplay()->drawBitmap(19, 18, CW_ICON_CANVAS, 27, 32, color);
  printCenter("- Canvas -", 7);
  printCenter(msg, 61);
}

void Clockface::update()
{
  if (!_is_setup) {
    if (millis() - lastMillis >= 5000) {
      drawSplashScreen(0xFFE0, "Retrying...");
      if (deserializeDefinition()) {
        clockfaceSetup();
        _is_setup = true;
      }
      lastMillis = millis();
    }
    return;
  }

  // Render animation
  clockfaceLoop();

  // Update Date/Time - Using a fixed interval (1000 milliseconds)
  if (millis() - lastMillis >= 1000)
  {
    refreshDateTime();
    lastMillis = millis();
  }
}

void Clockface::setFont(const char *fontName)
{
  if (fontName == nullptr)
  {
    Locator::getDisplay()->setFont();
  }
  else if (strcmp(fontName, "picopixel") == 0)
  {
    Locator::getDisplay()->setFont(&Picopixel);
  }
  else if (strcmp(fontName, "square") == 0)
  {
    Locator::getDisplay()->setFont(&atariFont);
  }
  else if (strcmp(fontName, "big") == 0)
  {
    Locator::getDisplay()->setFont(&hour8pt7b);
  }
  else if (strcmp(fontName, "medium") == 0)
  {
    Locator::getDisplay()->setFont(&minute7pt7b);
  }
  else
  {
    Locator::getDisplay()->setFont();
  }
}

void Clockface::renderText(String text, JsonVariantConst value)
{
  int16_t x1, y1;
  uint16_t w, h;

  const char* fontName = value["font"].as<const char *>();
  setFont(fontName);

  Locator::getDisplay()->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  // If the JSON provides an explicit bounding area width/height, use it so we fully erase old longer strings
  uint16_t bg_w = value.containsKey("width") ? value["width"].as<const uint16_t>() : w + max((int16_t)0, x1);
  uint16_t bg_h = value.containsKey("height") ? value["height"].as<const uint16_t>() : h + max((int16_t)0, y1);

  // BG Color
  Locator::getDisplay()->fillRect(
      value["x"].as<const uint16_t>(),
      value["y"].as<const uint16_t>() + min((int16_t)0, y1),
      bg_w,
      bg_h,
      value["bgColor"].as<const uint16_t>());

  Locator::getDisplay()->setTextColor(value["fgColor"].as<const uint16_t>());
  Locator::getDisplay()->setCursor(value["x"].as<const uint16_t>(), value["y"].as<const uint16_t>());
  Locator::getDisplay()->print(text);
}

void Clockface::refreshDateTime()
{
  JsonArrayConst elements = doc["setup"].as<JsonArrayConst>();
  for (JsonVariantConst value : elements)
  {
    const char *type = value["type"].as<const char *>();

    if (type != nullptr && strcmp(type, "datetime") == 0)
    {
      const char *format = value["format"].as<const char *>();
      if (format != nullptr && strlen(format) > 0) {
        renderText(_dateTime->getFormattedTime(format), value);
      } else {
        const char *content = value["content"].as<const char *>();
        if (content != nullptr) {
          renderText(_dateTime->getFormattedTime(content), value);
        }
      }
    }
  }
}

void Clockface::clockfaceSetup()
{
  // Clear screen
  Locator::getDisplay()->fillRect(0, 0, 64, 64, doc["bgColor"].as<const uint16_t>());

  delay = doc["delay"].as<const uint16_t>();

  // Draw static elements
  renderElements(doc["setup"].as<JsonArrayConst>());

  // Draw Date/Time
  refreshDateTime();

  // Create sprites
  createSprites();
}

void Clockface::createSprites()
{
  JsonArrayConst elements = doc["loop"].as<JsonArrayConst>();
  uint8_t width = 0;
  uint8_t height = 0;

  for (JsonVariantConst value : elements)
  {
    const char *type = value["type"].as<const char *>();

    if (type != nullptr && strcmp(type, "sprite") == 0)
    {
      uint8_t ref = value["sprite"].as<const uint8_t>();

      std::shared_ptr<CustomSprite> s = std::make_shared<CustomSprite>(value["x"].as<const int8_t>(), value["y"].as<const int8_t>());

      const char* image_b64 = doc["sprites"][ref][0]["image"].as<const char *>();
      if (image_b64 != nullptr) {
        getImageDimensions(image_b64, width, height);
      } else {
        width = 0; height = 0;
      }

      s.get()->_spriteReference = value["sprite"].as<const uint8_t>();
      s.get()->_totalFrames = doc["sprites"][ref].size();
      s.get()->setDimensions(width, height);
      sprites.push_back(s);
    }
  }
}

void Clockface::handleSpriteAnimation(std::shared_ptr<CustomSprite>& sprite) {
    uint8_t totalFrames = sprite->_totalFrames;
    uint32_t loopDelay = doc["loop"][sprite->_spriteReference]["loopDelay"] | delay;
    uint16_t frameDelay = doc["loop"][sprite->_spriteReference]["frameDelay"] | delay;

    if (millis() - sprite->_lastMillisSpriteFrames >= frameDelay && sprite->_currentFrameCount < totalFrames) {
        sprite->incFrame();

        // handle sprite movement
        handleSpriteMovement(sprite);

        // Render the frame of the sprite
        const char *img = doc["sprites"][sprite->_spriteReference][sprite->_currentFrame]["image"].as<const char *>();
        if (img != nullptr) {
            renderImage(img, sprite->getX(), sprite->getY());
        }

        sprite->_currentFrameCount += 1;
        sprite->_lastMillisSpriteFrames = millis();
    }

    if (millis() - sprite->_lastResetTime >= loopDelay) {
        unsigned long currentMillis = millis();
        unsigned long currentSecond = _dateTime->getSecond();

        if ((currentSecond * 1000) % loopDelay == 0) {
            sprite->_currentFrameCount = 0;
            sprite->_lastResetTime = currentMillis;
        }
    }
}

void Clockface::handleSpriteMovement(std::shared_ptr<CustomSprite>& sprite) {
    unsigned long moveStartTime = doc["loop"][sprite->_spriteReference]["moveStartTime"] | 1UL;
    unsigned long moveDuration = doc["loop"][sprite->_spriteReference]["moveDuration"] | 0UL;
    int8_t moveInitialX = doc["loop"][sprite->_spriteReference]["x"] | (int8_t)0;
    int8_t moveInitialY = doc["loop"][sprite->_spriteReference]["y"] | (int8_t)0;
    int8_t moveTargetX = doc["loop"][sprite->_spriteReference]["moveTargetX"] | (int8_t)-1;
    int8_t moveTargetY = doc["loop"][sprite->_spriteReference]["moveTargetY"] | (int8_t)-1;
    bool shouldReturnToOrigin = doc["loop"][sprite->_spriteReference]["shouldReturnToOrigin"] | false;

    // Check if the sprite is moving
    if (sprite->isMoving()) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - sprite->_moveStartTime;
        float progress = (static_cast<float>(elapsedTime) / sprite->_moveDuration);

        int8_t oldX = sprite->getX();
        int8_t oldY = sprite->getY();
        int8_t newX = sprite->lerp(sprite->_moveInitialX, sprite->_moveTargetX, progress);
        int8_t newY = sprite->lerp(sprite->_moveInitialY, sprite->_moveTargetY, progress);
        int8_t originX = min(oldX, newX);
        int8_t originY = min(oldY, newY);
        int8_t drawWidth = sprite->getWidth() + max(oldX, newX) - originX;
        int8_t drawHeight = sprite->getHeight() + max(oldY, newY) - originY;

        // Erase the previous position
        Locator::getDisplay()->fillRect(
            originX,
            originY,
            drawWidth,
            drawHeight,
            doc["bgColor"].as<const uint16_t>());

        if (progress <= 1) {
            // Update the sprite's position
            sprite->setX(newX);
            sprite->setY(newY);

        } else if (sprite->shouldReturnToOrigin()) {
            // Movement is complete
            sprite->setX(sprite->_moveTargetX);
            sprite->setY(sprite->_moveTargetY);

            if (!sprite->_isReversing) {
                sprite->reverseMoving(moveInitialX, moveInitialY);
            }
        } else {
            sprite->stopMoving();
        }
    }

    if ((moveDuration > 0 && (moveTargetX > -1 || moveTargetY > -1)) && (millis() - sprite->_lastResetMoveTime >= moveStartTime)) {
        unsigned long currentMillis = millis();
        unsigned long currentSecond = _dateTime->getSecond();

        if ((currentSecond * 1000) % moveStartTime == 0) {
            sprite->_lastResetMoveTime = currentMillis;
            sprite->startMoving(moveTargetX, moveTargetY, moveDuration, shouldReturnToOrigin);
        }
    }
}

void Clockface::clockfaceLoop() {
    if (sprites.empty()) {
        return;
    }

    for (auto& sprite : sprites) {
        handleSpriteAnimation(sprite);
    }
}

void Clockface::renderElements(JsonArrayConst elements)
{
  for (JsonVariantConst value : elements)
  {
    const char *type = value["type"].as<const char *>();
    if (type == nullptr) continue;

    if (strcmp(type, "text") == 0)
    {
      const char *content = value["content"].as<const char *>();
      if (content != nullptr) {
        renderText(content, value);
      }
    }
    else if (strcmp(type, "fillrect") == 0)
    {
      Locator::getDisplay()->fillRect(
          value["x"].as<const uint16_t>(),
          value["y"].as<const uint16_t>(),
          value["width"].as<const uint16_t>(),
          value["height"].as<const uint16_t>(),
          value["color"].as<const uint16_t>());
    }
    else if (strcmp(type, "rect") == 0)
    {
      Locator::getDisplay()->drawRect(
          value["x"].as<const uint16_t>(),
          value["y"].as<const uint16_t>(),
          value["width"].as<const uint16_t>(),
          value["height"].as<const uint16_t>(),
          value["color"].as<const uint16_t>());
    }
    else if (strcmp(type, "line") == 0)
    {
      Locator::getDisplay()->drawLine(
          value["x"].as<const uint16_t>(),
          value["y"].as<const uint16_t>(),
          value["x1"].as<const uint16_t>(),
          value["y1"].as<const uint16_t>(),
          value["color"].as<const uint16_t>());
    }
    else if (strcmp(type, "image") == 0)
    {
      const char *img = value["image"].as<const char *>();
      if (img != nullptr) {
        renderImage(img, value["x"].as<const uint8_t>(), value["y"].as<const uint8_t>());
      }
    }
  }
}

bool Clockface::deserializeDefinition()
{
  if (_server.empty() || _file.empty()) {
    drawSplashScreen(0xC904, "Params werent set");
    return false;
  }

  std::string url;
  if (_server.rfind("http://", 0) == 0 || _server.rfind("https://", 0) == 0) {
    url = _server + "/" + _file + ".json";
  } else {
    url = "http://" + _server + "/" + _file + ".json";
  }

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.timeout_ms = 10000;
  config.skip_cert_common_name_check = true;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    drawSplashScreen(0xC904, "Init failed");
    return false;
  }

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGCONFIG("Clockface", "Error opening HTTP connection: %s", esp_err_to_name(err));
    ESP_LOGCONFIG("Clockface", "Error code: %d", err);

    esp_http_client_cleanup(client);
    drawSplashScreen(0xC904, "Waiting for Net");
    return false;
  }

  int content_length = esp_http_client_fetch_headers(client);
  int status_code = esp_http_client_get_status_code(client);
  if (status_code != 200) {
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    ESP_LOGCONFIG("Clockface", "Failed URL: %s", url.c_str());\
    ESP_LOGCONFIG("Clockface", "Status Code: %d", status_code);
   
    drawSplashScreen(0xC904, "HTTP Error");
    return false;
  }

  std::string response_body;
  if (content_length > 0) {
    response_body.reserve(content_length);
  }
  
  char buffer[512];
  int read_len = 0;
  while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
    response_body.append(buffer, read_len);
  }

  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  DeserializationError error = deserializeJson(doc, response_body);
  if (error) {
    drawSplashScreen(0xC904, "JSON Error");
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }

  const char* nm = doc["name"].as<const char *>();
  const char* au = doc["author"].as<const char *>();
  Serial.printf("[Canvas] Building clockface '%s' by %s, version %d\n", nm ? nm : "Unknown", au ? au : "Unknown", doc["version"].as<const uint16_t>());
  return true;
}

} // namespace canvas
