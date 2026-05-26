#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>

constexpr int GAME_W = 1024;
constexpr int GAME_H = 768;
constexpr int CAM_W = 640;
constexpr int CAM_H = 360;

// Выносим переменную радиуса зайца, чтобы rabbit.cpp её видел
extern int calib_hitbox_radius; 
extern int calib_rabbit_orbit; // ВОЗВРАЩАЕМ ПЕРЕМЕННУЮ ОРБИТЫ СЮДА

struct Tree {
    int x, y;             
    float radius;         
    cv::Mat sprite;       
};
// ... (остальной код функции drawSprite оставляем)

// Отрисовка спрайтов
inline void drawSprite(cv::Mat& background, const cv::Mat& sprite, int x, int y) {
    if (sprite.empty()) return;
    for (int r = 0; r < sprite.rows; ++r) {
        int bg_r = y + r;
        if (bg_r < 0 || bg_r >= background.rows) continue;
        for (int c = 0; c < sprite.cols; ++c) {
            int bg_c = x + c;
            if (bg_c < 0 || bg_c >= background.cols) continue;
            
            if (sprite.channels() == 4) {
                cv::Vec4b alpha_pixel = sprite.at<cv::Vec4b>(r, c);
                float alpha = alpha_pixel[3] / 255.0f;
                if (alpha > 0.1f) {
                    cv::Vec3b bg_pixel = background.at<cv::Vec3b>(bg_r, bg_c);
                    background.at<cv::Vec3b>(bg_r, bg_c) = cv::Vec3b(
                        alpha_pixel[0] * alpha + bg_pixel[0] * (1.0f - alpha),
                        alpha_pixel[1] * alpha + bg_pixel[1] * (1.0f - alpha),
                        alpha_pixel[2] * alpha + bg_pixel[2] * (1.0f - alpha)
                    );
                }
            } else {
                background.at<cv::Vec3b>(bg_r, bg_c) = sprite.at<cv::Vec3b>(r, c);
            }
        }
    }
}
