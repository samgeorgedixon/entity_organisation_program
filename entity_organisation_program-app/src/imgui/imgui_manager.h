#pragma once
#include "sdl/SDL.h"
#include "imgui/imgui.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_impl_sdl2.h"

void ImGuiSetup(SDL_Window* window, SDL_Renderer* renderer);
void ImGuiEnd();

void ImGuiStartFrame();
void ImGuiRender(SDL_Window* window, SDL_Renderer* renderer);
