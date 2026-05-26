#include "rabbit.h"
#include "common.h"
#include <cstdlib>
#include <cmath>

Rabbit::Rabbit(float x, float y, const std::vector<cv::Mat>& sprites) 
    : m_x(x), m_y(y), m_width(100), m_height(100), m_sprites(sprites) {
    respawn();
}

void Rabbit::respawn() {
    m_x = rand() % (GAME_W - 300) + 150;
    m_y = rand() % (GAME_H - 300) + 150;
    
    m_vx = (rand() % 2 == 0 ? 2.5f : -2.5f);
    m_vy = (rand() % 2 == 0 ? 1.8f : -1.8f);
    m_anim_frame = 0;
}

void Rabbit::update(float wolf_center_x, float wolf_center_y, int score) {
    float rabbit_center_x = m_x + m_width / 2.0f;
    float rabbit_center_y = m_y + m_height / 2.0f;

    float dx = rabbit_center_x - wolf_center_x;
    float dy = rabbit_center_y - wolf_center_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Расчет прогрессивной физической скорости Зайца от счета
    float base_speed = 2.5f + (score * 1.0f);
    if (base_speed > 15.0f) base_speed = 15.0f;

    // === ДИНАМИЧЕСКИЙ РАСЧЕТ ЗОН ИИ ОТ РАЗМЕРА ХИТБОКСА ===
    // Минимальная дистанция касания колец
    float touch_distance = static_cast<float>(calib_hitbox_radius * 2); 
    
    // Критическая зона (укус/зажим): Заяц уже вплотную, даем фору на прорыв
    float critical_zone = touch_distance + 10.0f; 
    
    // Зона паники/погони: должна быть ощутимо больше, чтобы Заяц успел развернуться
    float scare_zone = touch_distance + 20.0f; 

    // ИИ страха перед Волком
    if (distance > 0.0f && distance < critical_zone) { 
        // Если Волк подошел ближе критической черты — Заяц включает режим паники 
        // и резко гасит скорость форсажа до 0.5х, давая Волку завершить укус в main.cpp,
        // но вектор направлен строго ОТ Волка, чтобы не бежать к нему в лапы самовольно
        if (distance > 0.0f) {
            m_vx = (dx / distance) * (base_speed * 0.5f); 
            m_vy = (dy / distance) * (base_speed * 0.5f);
        }
    }
    else if (distance >= critical_zone && distance < scare_zone) {
        // ЗОНА ПОГОНИ: Убегаем от Волка на форсаже (скорость x1.5)
        m_vx = (dx / distance) * (base_speed * 1.5f); 
        m_vy = (dy / distance) * (base_speed * 1.5f);
    } 
    else {
        // ВОЛК ДАЛЕКО: Спокойный патруль по текущему курсу
        float current_speed = std::sqrt(m_vx * m_vx + m_vy * m_vy);
        if (current_speed > 0) {
            m_vx = (m_vx / current_speed) * base_speed;
            m_vy = (m_vy / current_speed) * base_speed;
        }
    }

    // Физическое смещение
    m_x += m_vx;
    m_y += m_vy;
    
    rabbit_center_x = m_x + m_width / 2.0f;
    rabbit_center_y = m_y + m_height / 2.0f;

    // === ГЕОМЕТРИЯ ЭЛЛИПСА И ЧЕСТНЫЙ РИКОШЕТ ===
    float center_screen_x = GAME_W / 2.0f;
    float center_screen_y = GAME_H / 2.0f;

    // Границы орбиты Зайца по размерам окна
    float ellipse_a = (GAME_W / 2.0f) - 60.0f; 
    float ellipse_b = (GAME_H / 2.0f) - 60.0f; 

    float rel_x = rabbit_center_x - center_screen_x;
    float rel_y = rabbit_center_y - center_screen_y;
    float ellipse_value = (rel_x * rel_x) / (ellipse_a * ellipse_a) + (rel_y * rel_y) / (ellipse_b * ellipse_b);

    if (ellipse_value > 1.0f) {
        float nx = (2.0f * rel_x) / (ellipse_a * ellipse_a);
        float ny = (2.0f * rel_y) / (ellipse_b * ellipse_b);
        float n_len = std::sqrt(nx * nx + ny * ny);
        
        if (n_len > 0.0f) {
            nx /= n_len; 
            ny /= n_len;

            float dot_product = m_vx * nx + m_vy * ny;
            
            if (dot_product > 0.0f) {
                float reflect_vx = m_vx - 2.0f * dot_product * nx;
                float reflect_vy = m_vy - 2.0f * dot_product * ny;

                float rand_angle = ((rand() % 50 - 25) * 3.14159f) / 180.0f;
                
                m_vx = (reflect_vx * std::cos(rand_angle) - reflect_vy * std::sin(rand_angle));
                m_vy = (reflect_vx * std::sin(rand_angle) + reflect_vy * std::cos(rand_angle));

                float new_speed = std::sqrt(m_vx * m_vx + m_vy * m_vy);
                if (new_speed > 0.0f) {
                    m_vx = (m_vx / new_speed) * base_speed;
                    m_vy = (m_vy / new_speed) * base_speed;
                }

                m_x -= nx * 8.0f;
                m_y -= ny * 8.0f;
            }
        }
    }

    // Системные барьеры
    if (m_x < -200) { m_vx = std::abs(m_vx); m_x = -190; }
    if (m_x > GAME_W + 200) { m_vx = -std::abs(m_vx); m_x = GAME_W + 190; }
    if (m_y < -200) { m_vy = std::abs(m_vy); m_y = -190; }
    if (m_y > GAME_H + 200) { m_vy = -std::abs(m_vy); m_y = GAME_H + 190; }

    // Анимация спрайтов
    m_anim_tick++;
    int anim_speed_divider = 6 - (score / 2); 
    if (anim_speed_divider < 2) anim_speed_divider = 2; 

    if (m_anim_tick % anim_speed_divider == 0) {
        m_anim_frame = (m_anim_frame + 1) % 4; 
    }
}

void Rabbit::draw(cv::Mat& canvas) {
    if (m_sprites.empty()) return;
    cv::Mat current = m_sprites[m_anim_frame].clone();
    if (m_vx < 0) cv::flip(current, current, 1);
    drawSprite(canvas, current, static_cast<int>(m_x), static_cast<int>(m_y));
}

cv::Rect Rabbit::getHitbox() const {
    return cv::Rect(static_cast<int>(m_x), static_cast<int>(m_y), m_width, m_height);
}
