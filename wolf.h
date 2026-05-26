#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class Wolf {
private:
    float m_x, m_y;
    float m_vx, m_vy;
    int m_width, m_height;
    int m_anim_frame = 0;
    int m_anim_tick = 0;
    bool m_is_running = false;
    float m_target_x, m_target_y, m_base_speed, m_current_speed;
    const float m_friction = 0.95f;
    std::vector<cv::Mat> m_sprites;

public:
    Wolf(float x, float y, const std::vector<cv::Mat>& sprites);
    void applyImpulse(int shot_x, int shot_y);
    void update();
    void handleCollisionWithBush(int bush_x, int bush_y, float bush_radius);
    void draw(cv::Mat& canvas);
    void resetPosition();
    cv::Rect getHitbox() const;
};
