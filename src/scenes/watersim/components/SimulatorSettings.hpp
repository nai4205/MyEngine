#pragma once

struct SimulatorSettings {
  float particleRadius = 10.0f;
  float particleSpacing = 10.0f;
  int numParticles = 1000;
  float gravity = -500.0f;
  float dampFactor = 0.8f;
  float boxMargin = 50.0f;
  float wallThickness = 20.0f;
};
