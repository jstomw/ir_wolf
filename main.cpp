#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <algorithm>
#include "common.h"
#include "wolf.h"
#include "rabbit.h"
#include "map_manager.h"

// Глобальные переменные калибровки (загружаются из config.txt)
std::string device_id = "";
int threshold_value = 100; 
int calib_radial_mult = 40; 
int show_hud_value = 0;     
int calib_hitbox_radius = 25; 
int calib_rabbit_orbit = 0; // ВОЗВРАЩАЕМ ОРБИТУ ПО УМОЛЧАНИЮ (0)

// Функция для безопасной загрузки и очистки серого фона у картинок
inline cv::Mat loadAndCleanSprite(const std::string& path) {
    cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "[ВНИМАНИЕ] Не найден файл: " << path << ". Использую пустышку." << std::endl;
        return cv::Mat::zeros(100, 100, CV_8UC4);
    }
    
    cv::Mat bgra;
    if (img.channels() == 3) {
        cv::cvtColor(img, bgra, cv::COLOR_BGR2BGRA);
    } else {
        bgra = img.clone();
    }

    // Хромакей: убираем темно-серый фон спрайтов
    for (int i = 0; i < bgra.rows; ++i) {
        for (int j = 0; j < bgra.cols; ++j) {
            cv::Vec4b& p = bgra.at<cv::Vec4b>(i, j);
            if (p[0] < 55 && p[1] < 55 && p[2] < 55) {
                p[3] = 0; // Делаем прозрачным альфа-канал
            }
        }
    }
    return bgra;
}

// Загрузка файла конфигурации
void loadConfig() {
    std::ifstream file("data/config.txt");
    if (!file.is_open()) {
        std::cout << "[CONFIG] Файл настроек не найден, использую дефолтные." << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.rfind("#", 0) == 0) continue;
        size_t delimiter = line.find('=');
        if (delimiter != std::string::npos) {
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);
            if (key == "Threshold") threshold_value = std::stoi(value);
            else if (key == "Multiplier") calib_radial_mult = std::stoi(value);
            else if (key == "ShowHUD") show_hud_value = std::stoi(value);
            else if (key == "DeviceID") device_id = value;
            else if (key == "HitboxRadius") calib_hitbox_radius = std::stoi(value);
            else if (key == "RabbitOrbit") calib_rabbit_orbit = std::stoi(value); // Читаем орбиту
        }
    }
    file.close();
    std::cout << "[CONFIG] Настройки успешно загружены!" << std::endl;
}

// Сохранение файла конфигурации
void saveConfig() {
    std::ofstream file("data/config.txt");
    if (file.is_open()) {
        file << "Threshold=" << threshold_value << "\n";
        file << "Multiplier=" << calib_radial_mult << "\n";
        file << "ShowHUD=" << show_hud_value << "\n";
        file << "DeviceID=" << device_id << "\n";
        file << "HitboxRadius=" << calib_hitbox_radius << "\n";
        file << "RabbitOrbit=" << calib_rabbit_orbit << "\n"; // Сохраняем орбиту
        file.close();
        std::cout << "[CONFIG] Настройки сохранены!" << std::endl;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    loadConfig();

    std::vector<cv::Mat> wolf_frames = {
        loadAndCleanSprite("data/wolf_sit.png"),
        loadAndCleanSprite("data/wolf_run1.png"),
        loadAndCleanSprite("data/wolf_run2.png"),
        loadAndCleanSprite("data/wolf_run3.png"),
        loadAndCleanSprite("data/wolf_run4.png")
    };

    std::vector<cv::Mat> hare_frames = {
        loadAndCleanSprite("data/hare1.png"),
        loadAndCleanSprite("data/hare2.png"),
        loadAndCleanSprite("data/hare3.png"),
        loadAndCleanSprite("data/hare4.png")
    };

    cv::Mat t1 = loadAndCleanSprite("data/tree1.png");
    cv::Mat t2 = loadAndCleanSprite("data/tree2.png");
    cv::Mat t3 = loadAndCleanSprite("data/tree3.png");
    cv::Mat claw_sprite = loadAndCleanSprite("data/wolf_claw.png");

    for (auto& f : wolf_frames) cv::resize(f, f, cv::Size(120, 120));
    for (auto& f : hare_frames) cv::resize(f, f, cv::Size(100, 100));
    cv::resize(t1, t1, cv::Size(100, 100));
    cv::resize(t2, t2, cv::Size(100, 100));
    cv::resize(t3, t3, cv::Size(100, 100));
    cv::resize(claw_sprite, claw_sprite, cv::Size(40, 40));

    std::string final_device = device_id.empty() ? "/dev/video2" : device_id;
    cv::VideoCapture cap(final_device, cv::CAP_V4L2);
    if (!cap.isOpened()) return -1;
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH, CAM_W);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_H);

    cv::namedWindow("Nu, Pogodi! Radial Mapping Edition", cv::WINDOW_AUTOSIZE);
    
    cv::namedWindow("CALIBRATION PANEL", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("IR Threshold", "CALIBRATION PANEL", &threshold_value, 255);
    cv::createTrackbar("Radial Multiplier", "CALIBRATION PANEL", &calib_radial_mult, 100); 
    cv::createTrackbar("Show HUD (0/1)", "CALIBRATION PANEL", &show_hud_value, 1);
    cv::createTrackbar("Hitbox Radius", "CALIBRATION PANEL", &calib_hitbox_radius, 60);
    // ВОЗВРАЩАЕМ ПОЛЗУНОК ОРБИТЫ В GUI
    cv::createTrackbar("Rabbit Orbit", "CALIBRATION PANEL", &calib_rabbit_orbit, 300);

    Wolf wolf(GAME_W / 2 - 60, GAME_H / 2 - 60, wolf_frames);
    Rabbit rabbit(100, 100, hare_frames);

    MapManager mapManager(t1, t2, t3, 120.0f);
    mapManager.generateMap(0);

    int score = 0;
    cv::Mat cam_frame, thresh;
    cv::Mat canvas = cv::Mat::zeros(GAME_H, GAME_W, CV_8UC3);
    
    int target_x = GAME_W / 2;
    int target_y = GAME_H / 2;
    int raw_cam_view_x = GAME_W / 2;
    int raw_cam_view_y = GAME_H / 2;
    int camera_warmup_frames = 0;
    bool trackbar_window_visible = true;

    while (true) {
        canvas.setTo(cv::Scalar(45, 45, 45)); 

        rabbit.update(wolf.getHitbox().x + 60, wolf.getHitbox().y + 60, score);
        wolf.update();

        mapManager.checkWolfCollisions(wolf);

        // Координаты центров персонажей
        cv::Rect w_box = wolf.getHitbox();
        cv::Rect r_box = rabbit.getHitbox();
        float w_cx = w_box.x + w_box.width / 2.0f;
        float w_cy = w_box.y + w_box.height / 2.0f;
        float r_cx = r_box.x + r_box.width / 2.0f;
        float r_cy = r_box.y + r_box.height / 2.0f;

        // === ХАК СНАЙПЕРСКОГО УПРЕЖДЕНИЯ (LOOK-AHEAD) ===
        // Берем текущие скорости Зайца из его объекта. 
        // Чтобы main.cpp видел векторы скоростей, мы вызовем геттеры (или используем упреждение по направлению)
        // Считаем дистанцию с упреждением: где Заяц окажется через полкадра
        float future_r_cx = r_cx + (rabbit.getVx() * 0.5f);
        float future_r_cy = r_cy + (rabbit.getVy() * 0.5f);

        float dist_to_rabbit = std::sqrt((w_cx - r_cx)*(w_cx - r_cx) + (w_cy - r_cy)*(w_cy - r_cy));
        float dist_future = std::sqrt((w_cx - future_r_cx)*(w_cx - future_r_cx) + (w_cy - future_r_cy)*(w_cy - future_r_cy));

        // ПОИМКА: Засчитывается, если Волк коснулся текущего тела ИЛИ перехватил Зайца на шаге упреждения
        // Радиус коллизии делаем чуть более отзывчивым: (два радиуса хитбокса + 4 пикселя форы на укус)
        if (dist_to_rabbit < (calib_hitbox_radius * 2.0f + 4.0f) || dist_future < (calib_hitbox_radius * 2.0f)) {
            score++;
            wolf.resetPosition();
            rabbit.respawn();
            mapManager.generateMap(score);
            target_x = GAME_W / 2;
            target_y = GAME_H / 2;
        }


        cap >> cam_frame;
        if (!cam_frame.empty()) {
            if (cam_frame.channels() == 3) cv::cvtColor(cam_frame, cam_frame, cv::COLOR_BGR2GRAY);
            cv::threshold(cam_frame, thresh, threshold_value, 255, cv::THRESH_BINARY);
            
            if (show_hud_value == 1) {
                cv::imshow("CALIBRATION PANEL", thresh);
                trackbar_window_visible = true;
            } else {
                if (trackbar_window_visible) {
                    try { cv::destroyWindow("CALIBRATION PANEL"); } catch(...) {}
                    trackbar_window_visible = false;
                }
            }

            if (camera_warmup_frames < 10) {
                camera_warmup_frames++;
            } else {
                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                for (const auto& contour : contours) {
                    if (cv::contourArea(contour) > 3) {
                        cv::Moments m = cv::moments(contour);
                        if (m.m00 > 0) {
                            int cam_x = static_cast<int>(m.m10 / m.m00);
                            int cam_y = static_cast<int>(m.m01 / m.m00);
                            cam_x = CAM_W - cam_x; 

                            float cam_center_x = CAM_W / 2.0f;
                            float cam_center_y = CAM_H / 2.0f;
                            float dx = cam_x - cam_center_x;
                            float dy = cam_y - cam_center_y;
                            float raw_radius = std::sqrt(dx * dx + dy * dy);

                            if (raw_radius > 0.0f) {
                                raw_cam_view_x = static_cast<int>((GAME_W / 2) + dx * 1.6f);
                                raw_cam_view_y = static_cast<int>((GAME_H / 2) + dy * 1.6f);

                                float radial_multiplier = static_cast<float>(calib_radial_mult) / 10.0f;
                                float new_radius = raw_radius * radial_multiplier;
                                float scale_factor = 1.6f;

                                int new_target_x = static_cast<int>((GAME_W / 2) + (dx / raw_radius) * new_radius * scale_factor);
                                int new_target_y = static_cast<int>((GAME_H / 2) + (dy / raw_radius) * new_radius * scale_factor);

                                new_target_x = std::max(0, std::min(GAME_W, new_target_x));
                                new_target_y = std::max(0, std::min(GAME_H, new_target_y));

                                target_x = target_x + static_cast<int>((new_target_x - target_x) * 0.3f);
                                target_y = target_y + static_cast<int>((new_target_y - target_y) * 0.3f);

                                wolf.applyImpulse(target_x, target_y);
                            }
                        }
                    }
                }
            }
        }

        rabbit.draw(canvas);
        wolf.draw(canvas);
        mapManager.drawTrees(canvas);

        if (show_hud_value == 1) {
            float m_factor = static_cast<float>(calib_radial_mult) / 10.0f;
            if (m_factor < 0.1f) m_factor = 0.1f;
            int max_visible_radius = static_cast<int>((GAME_W / 2) / m_factor);

            cv::circle(canvas, cv::Point(GAME_W / 2, GAME_H / 2), max_visible_radius, cv::Scalar(0, 180, 0), 1, cv::LINE_AA);
            
            // ДИНАМИЧЕСКИЙ СИНИЙ ЭЛЛИПС ПО НОВОЙ РАСШИРЯЕМОЙ ФОРМУЛЕ
            float ellipse_a = (GAME_W / 2.0f) - 60.0f + static_cast<float>(calib_rabbit_orbit);
            float ellipse_b = (GAME_H / 2.0f) - 60.0f + static_cast<float>(calib_rabbit_orbit);
            cv::ellipse(canvas, cv::Point(GAME_W/2, GAME_H/2), cv::Size(static_cast<int>(ellipse_a), static_cast<int>(ellipse_b)), 0, 0, 360, cv::Scalar(150, 100, 50), 1, cv::LINE_AA);

            cv::circle(canvas, cv::Point(raw_cam_view_x, raw_cam_view_y), 5, cv::Scalar(255, 0, 0), -1, cv::LINE_AA);
            cv::line(canvas, cv::Point(GAME_W / 2, GAME_H / 2), cv::Point(raw_cam_view_x, raw_cam_view_y), cv::Scalar(100, 100, 100), 1, cv::LINE_AA);
            cv::line(canvas, cv::Point(raw_cam_view_x, raw_cam_view_y), cv::Point(target_x, target_y), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
            
            cv::circle(canvas, cv::Point(w_cx, w_cy), calib_hitbox_radius, cv::Scalar(0, 0, 255), 1, cv::LINE_AA); 
            cv::circle(canvas, cv::Point(r_cx, r_cy), calib_hitbox_radius, cv::Scalar(0, 0, 255), 1, cv::LINE_AA); 

            cv::drawMarker(canvas, cv::Point(target_x, target_y), cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 15, 2);
        } else {
            drawSprite(canvas, claw_sprite, target_x - 20, target_y - 20);
        }

        cv::putText(canvas, "SCORE: " + std::to_string(score), cv::Point(30, 50), cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        cv::imshow("Nu, Pogodi! Radial Mapping Edition", canvas);

        if (cv::waitKey(30) == 27) {
            saveConfig(); 
            break; 
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
