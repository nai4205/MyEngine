#pragma once

struct PostProcessingComponent {
  bool shake = false;
  bool confuse = false;
  bool chaos = false;

  // Shake parameters
  float shakeTime = 0.0f;
  float shakeDuration = 0.05f;
};
