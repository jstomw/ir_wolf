#include "wolf.h"
#include "common.h"
#include <cmath>

Wolf::Wolf(float x, float y, const std::vector<cv::Mat>& sprites) 
    : m_x(x), m_y(y), m_width(120), m_height(120), m_sprites(sprites) {
    m_vx = 0.0f; 
    m_vy = 0.0f;
    m_base_speed = 3.5f; 
    m_current_speed = 0.0f;
    
    // ИСПРАВЛЕНИЕ 1 & 2: При старте цель строго там же, где Волк. Бег выключен.
    m_target_x = m_x + m_width / 2.0f;
    m_target_y = m_y + m_height / 2.0f;
    m_is_running = false; 
    m_anim_frame = 0; // Сразу сидит на жопе
}

void Wolf::applyImpulse(int shot_x, int shot_y) {
    float center_x = m_x + m_width / 2.0f;
    float center_y = m_y + m_height / 2.0f;

    float target_dx = shot_x - m_target_x;
    float target_dy = shot_y - m_target_y;
    float target_dist = std::sqrt(target_dx * target_dx + target_dy * target_dy);

    // Подгазовка (клик в радиусе 60 пикселей от текущей цели)
    if (m_is_running && target_dist < 60.0f) {
        m_current_speed += 1.5f; 
        if (m_current_speed > 12.0f) m_current_speed = 12.0f; 
    } else {
        m_current_speed = m_base_speed;
    }

    m_target_x = shot_x;
    m_target_y = shot_y;
    m_is_running = true;
}

void Wolf::update() {
    // Если флаг бега опущен — жестко зануляем скорости и включаем спрайт "сидит"
    if (!m_is_running) {
        m_vx = 0.0f; 
        m_vy = 0.0f;
        m_current_speed = 0.0f;
        m_anim_frame = 0; 
        return;
    }

    float center_x = m_x + m_width / 2.0f;
    float center_y = m_y + m_height / 2.0f;
    
    float dx = m_target_x - center_x;
    float dy = m_target_y - center_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Если добежали до цели — садимся на жопу
    if (distance < 8.0f) {
        m_is_running = false;
        return;
    }

    m_vx = (dx / distance) * m_current_speed;
    m_vy = (dy / distance) * m_current_speed;

    m_x += m_vx;
    m_y += m_vy;

    // Проверка границ экрана
    if (m_x < 0) { m_x = 0; m_is_running = false; }
    if (m_x > GAME_W - m_width) { m_x = GAME_W - m_width; m_is_running = false; }
    if (m_y < 0) { m_y = 0; m_is_running = false; }
    if (m_y > GAME_H - m_height) { m_y = GAME_H - m_height; m_is_running = false; }

    // Анимация бега (кадры 1..4)
    m_anim_tick++;
    if (m_anim_tick % 5 == 0) {
        m_anim_frame = 1 + (m_anim_frame % 4); 
    }
}

// ИСПРАВЛЕНИЕ 3: Исправляем тупняк в кустах
void Wolf::handleCollisionWithBush(int bush_x, int bush_y, float bush_radius) {
    float center_x = m_x + m_width / 2.0f;
    float center_y = m_y + m_height / 2.0f;
    float wolf_radius = 40.0f; 

    float dx = center_x - bush_x;
    float dy = center_y - bush_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < (wolf_radius + bush_radius)) {
        // 1. Чуть-чуть выталкиваем волка из куста обратно, чтобы хитбоксы не слипались
        m_x += (dx / distance) * 6.0f;
        m_y += (dy / distance) * 6.0f;
        
        // 2. Сбрасываем цель прямо под текущую позицию Волка
        m_target_x = m_x + m_width / 2.0f;
        m_target_y = m_y + m_height / 2.0f;
        
        // 3. Гасим стейт бега — Волк мгновенно садится на жопу перед препятствием
        m_is_running = false; 
    }
}

void Wolf::draw(cv::Mat& canvas) {
    if (m_sprites.empty()) return;
    cv::Mat current = m_sprites[m_anim_frame].clone();
    if (m_vx < 0) cv::flip(current, current, 1);
    drawSprite(canvas, current, static_cast<int>(m_x), static_cast<int>(m_y));
}

void Wolf::resetPosition() {
    m_x = GAME_W / 2 - 60;
    m_y = GAME_H / 2 - 60;
    m_target_x = GAME_W / 2;
    m_target_y = GAME_H / 2;
    m_vx = 0.0f; 
    m_vy = 0.0f;
    m_current_speed = 0.0f;
    m_is_running = false;
    m_anim_frame = 0;
}

cv::Rect Wolf::getHitbox() const {
    return cv::Rect(static_cast<int>(m_x), static_cast<int>(m_y), m_width, m_height);
}
