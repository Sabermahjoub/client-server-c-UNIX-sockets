#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "raylib.h"
#include <string.h>
#define BUFFER_SIZE 1024

// Important: Include raylib.h before defining RAYGUI_IMPLEMENTATION
#include "../../external/raygui/src/raygui.h"

// Authentication states
typedef enum {
    AUTH_LOGIN,
    AUTH_SUCCESS,
    AUTH_FAILURE,
    AUTH_LOADING
} AuthState;

// Function to simulate authentication (replace with real authentication logic)
bool authenticateUser(const char* buffer) {
    if (strcmp(buffer, "AUTH_OK") != 0) {
        printf("Invalid credentials / Authentification échouée.\n");
        return false;
    }
    else {
        printf("Successful authentication / authentification réussie.\n");
        return true;
    }
}

int authenticate(int sock)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Authentication System");

    // State variables
    AuthState currentState = AUTH_LOGIN;
    float messageTimer = 0.0f;
    bool showPassword = false;
    
    // Input buffers and states
    char username[50] = "";
    char password[50] = "";
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int attempt = 0;
    char errorMessage[100] = "";
    bool usernameEditMode = false;
    bool passwordEditMode = false;
    
    // Loading animation variables
    float loadingAngle = 0.0f;
    
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose() && attempt <3)
    {
        // Update
        if (currentState == AUTH_LOADING) {
            loadingAngle += 5.0f;
            if (loadingAngle >= 360.0f) loadingAngle = 0.0f;
        }
        
        if (messageTimer > 0) {
            messageTimer -= GetFrameTime();
            if (messageTimer <= 0) {
                if (currentState == AUTH_FAILURE) {
                    currentState = AUTH_LOGIN;
                }
            }
        }

        // Draw
        BeginDrawing();
        
        int screen_width = GetRenderWidth();
        int screen_height = GetRenderHeight();
        
        ClearBackground(DARKGRAY);
        
        // Draw different screens based on state
        switch(currentState) {
            case AUTH_LOGIN:
            {   
                // Title
                DrawText("Authentication System", screen_width / 4.5, 
                        screen_height / 10, screen_height/10, RAYWHITE);

                char attemps_left[50] = "Attemps lefts : ";
                char str[50];
                sprintf(str, "%d", 3 - attempt) ;
                strcat(attemps_left, str);

                DrawText(attemps_left, screen_width / 2.4, 
                        screen_height / 5, screen_height/25, ORANGE);                
                // Username input
                Rectangle username_input_box = {
                    (screen_width-0.3*screen_width)/2,
                    1.3*screen_height/7+screen_height*0.2,
                    screen_width*0.3,
                    screen_height*0.07
                };

                DrawText("Username:", username_input_box.x, 
                        username_input_box.y - screen_height/20, screen_height/20, RAYWHITE);
                
                // Handle username input
                if (GuiTextBox(username_input_box, username, 250, usernameEditMode)) {
                    usernameEditMode = !usernameEditMode;
                    passwordEditMode = false;  // Ensure only one field is active at a time
                }

                // Password input
                Rectangle pwd_input_box = {
                    username_input_box.x,
                    username_input_box.y + screen_height*0.2,
                    username_input_box.width,
                    username_input_box.height
                };
                DrawText("Password:", pwd_input_box.x, 
                        pwd_input_box.y - screen_height/20, screen_height/20, RAYWHITE);
                
                // Password visibility toggle
                Rectangle showPwdBox = {
                    pwd_input_box.x + pwd_input_box.width + 10,
                    pwd_input_box.y,
                    30,
                    pwd_input_box.height
                };
                if (GuiButton(showPwdBox, showPassword ? "Not" : "Show")) {
                    showPassword = !showPassword;
                }
                
                // Handle password input
                char displayPassword[250] = "";
                if (!showPassword) {
                    memset(displayPassword, '*', strlen(password));
                } else {
                    strcpy(displayPassword, password);
                }
                
                if (GuiTextBox(pwd_input_box, displayPassword, 250, passwordEditMode)) {
                    passwordEditMode = !passwordEditMode;
                    usernameEditMode = false;  // Ensure only one field is active at a time
                }
                
                // Update password buffer only when in edit mode
                if (passwordEditMode) {
                    strcpy(password, displayPassword);
                }

                // Login button with hover effect
                Rectangle loginBtn = {
                    pwd_input_box.x,
                    pwd_input_box.y + screen_height*0.2,
                    username_input_box.width,
                    username_input_box.height
                };
                
                if (GuiButton(loginBtn, "Login"))
                {
                    if (strlen(username) == 0 || strlen(password) == 0) {
                        strcpy(errorMessage, "Please fill in all fields");
                        currentState = AUTH_FAILURE;
                        messageTimer = 3.0f;
                    } else {
                        currentState = AUTH_LOADING;
                        snprintf(buffer, BUFFER_SIZE, "%s:%s", username, password);
                        send(sock, buffer, strlen(buffer), 0);

                        memset(buffer, 0, BUFFER_SIZE);
                        // Vérification de l'authentification -- Authentication verification
                        recv(sock, buffer, BUFFER_SIZE-1, 0);
                        buffer[BUFFER_SIZE] = '\0';
                        if (authenticateUser(buffer)) {
                            currentState = AUTH_SUCCESS;
                        } else {
                            attempt++;
                            printf("Tentative/Attempt n°: %d \n", attempt);
                            if (attempt == 3){
                                printf("Nombre de tentatives est dépassé / Number of attempts is exceeded :\n");
                                close(sock);
                                exit(EXIT_FAILURE);
                            }
                            strcpy(errorMessage, "Invalid credentials");
                            currentState = AUTH_FAILURE;
                            messageTimer = 3.0f;
                        }


                    }
                }
                
                // Draw error message if any
                if (strlen(errorMessage) > 0) {
                    DrawText(errorMessage, screen_width/2 - MeasureText(errorMessage, 20)/2,
                            loginBtn.y + loginBtn.height + 20, 20, RED);
                }

                // Tab key handling for field switching
                if (IsKeyPressed(KEY_TAB)) {
                    if (usernameEditMode) {
                        usernameEditMode = false;
                        passwordEditMode = true;
                    } else if (passwordEditMode) {
                        passwordEditMode = false;
                        usernameEditMode = true;
                    } else {
                        usernameEditMode = true;
                    }
                }
            } break;
            
            case AUTH_LOADING:
            {
                // Draw loading spinner
                DrawText("Authenticating...", screen_width/2 - MeasureText("Authenticating...", 30)/2,
                        screen_height/2 - 50, 30, RAYWHITE);
                
                DrawCircleSector((Vector2){screen_width/2, screen_height/2 + 50},
                               30, loadingAngle, loadingAngle + 300, 0, RAYWHITE);
            } break;
            
            case AUTH_SUCCESS:
            {
                DrawText("Login Successful!", screen_width/2 - MeasureText("Login Successful!", 40)/2,
                        screen_height/2 - 20, 40, GREEN);
                DrawText("Press ESC to exit", screen_width/2 - MeasureText("Press ESC to exit", 20)/2,
                        screen_height/2 + 40, 20, RAYWHITE);
                return 1;
            } break;
            
            case AUTH_FAILURE:
            {
                DrawText(errorMessage, screen_width/2 - MeasureText(errorMessage, 30)/2,
                        screen_height/2 - 15, 30, RED);
            } break;
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}