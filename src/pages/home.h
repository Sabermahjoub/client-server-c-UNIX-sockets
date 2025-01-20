#pragma once

#include "raylib.h"
#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int home(int sock){

    // Initialization
    const int screen_Width = 1200;    // Increased width for response area
    const int screen_Height = 600;

    // Client choice
    int choice;
    char buffer[BUFFER_SIZE];
    char displayResponse[BUFFER_SIZE] = "Server response will appear here...";
    
    const Color BACKGROUND_COLOR = (Color){ 28, 36, 56, 255 };     // Dark blue background
    const Color TITLE_COLOR = (Color){ 238, 242, 255, 255 };       // Bright white
    const Color MENU_ITEM_COLOR = (Color){ 189, 197, 227, 255 };   // Light blue-gray
    const Color HIGHLIGHT_COLOR = (Color){ 64, 159, 255, 255 };    // Bright blue
    const Color SEPARATOR_COLOR = (Color){ 45, 55, 72, 255 };      // Slightly lighter than background
    const Color RESPONSE_BG = (Color){ 22, 28, 44, 255 };          // Darker background for response area

    InitWindow(screen_Width, screen_Height, "Client Menu");
    SetTargetFPS(60);

    // Track active menu item
    int activeChoice = -1;

    while (!WindowShouldClose()){

        int screenWidth = GetRenderWidth();
        int screenHeight = GetRenderHeight();
        Vector2 mousePoint = GetMousePosition();
        
        // Check for hover states
        float menuStartY = screenHeight * 0.35f;
        float menuSpacing = screenHeight * 0.15f;
        Rectangle menuItems[4];
        
        for (int i = 0; i < 4; i++) {
            menuItems[i] = (Rectangle){
                screen_Width * 0.05f,                // Moved left for response area
                menuStartY + (i * menuSpacing),
                screen_Width * 0.4f,                 // Reduced width for menu items
                screen_Height * 0.08f
            };
            
            if (CheckCollisionPointRec(mousePoint, menuItems[i])) {
                activeChoice = i;
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    choice = i+1;
                    // Communicate with server through load balancer
                    sprintf(buffer, "%d", choice);
                    send(sock, buffer, strlen(buffer), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    int bytes_received = read(sock, buffer, BUFFER_SIZE);
                    buffer[bytes_received] = '\0';
                    strncpy(displayResponse, buffer, BUFFER_SIZE - 1);  // Update display response
                    printf("Server replied: %s\n", buffer);
                }
            }
        }

        BeginDrawing();
        {
            ClearBackground(BACKGROUND_COLOR);
            
            // Draw decorative header bar
            DrawRectangle(0, 0, screenWidth, screenHeight * 0.2f, SEPARATOR_COLOR);
            
            // Title with shadow effect
            const char* title = "Client Menu";
            int titleSize = screenHeight/8;
            Vector2 titlePos = (Vector2){
                screenWidth * 0.25f - MeasureText(title, titleSize) * 0.5f,  // Moved left
                screenHeight * 0.04f
            };

            // Title
            DrawText(title, titlePos.x + 3, titlePos.y + 3, titleSize, (Color){0, 0, 0, 100});
            DrawText(title, titlePos.x, titlePos.y, titleSize, TITLE_COLOR);

            // Menu items
            const char* menuTexts[] = {
                "Get Today's Date",
                "Get All Files",
                "Get File's Content",
                "Get Elapsed Time"
            };

            for (int i = 0; i < 4; i++) {
                if (i == activeChoice) {
                    DrawRectangleRec(menuItems[i], SEPARATOR_COLOR);
                    DrawRectangleLinesEx(menuItems[i], 1, HIGHLIGHT_COLOR);
                }

                char menuItem[50];
                sprintf(menuItem, "%d. %s", i + 1, menuTexts[i]);
                
                Vector2 textPos = {
                    menuItems[i].x + 20,
                    menuItems[i].y + (menuItems[i].height - screen_Height/20) * 0.5f
                };
                
                DrawText(menuItem,
                        textPos.x,
                        textPos.y,
                        screen_Height/20,
                        i == activeChoice ? HIGHLIGHT_COLOR : MENU_ITEM_COLOR);
            }

            // Response display area (right side)
            Rectangle responseArea = {
                screenWidth * 0.5f,      
                screenHeight * 0.25f,           
                screenWidth * 0.45f,           
                screenHeight * 0.7f             
            };
            
            // Draw response area
            DrawRectangleRec(responseArea, RESPONSE_BG);
            DrawRectangleLinesEx(responseArea, 1, SEPARATOR_COLOR);
            
            // Response area title
            DrawText("Server Response", 
                    responseArea.x + 20, 
                    responseArea.y + 20, 
                    screenHeight/30, 
                    TITLE_COLOR);
            
            // Draw actual response
            DrawText(TextSubtext(displayResponse, 0, strlen(displayResponse)),
                    responseArea.x + 20,
                    responseArea.y + 60,
                    screenHeight/30,
                    MENU_ITEM_COLOR);

            // Bottom decoration
            DrawRectangle(0, screenHeight - 4, screenWidth, 5, SEPARATOR_COLOR);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}