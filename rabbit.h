#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class Rabbit {
private:
    float m_x, m_y;
    float m_vx, m_vy;
    int m_width, m_height;
    int m_anim_frame = 0;
    int m_anim_tick = 0;
    std::vector<cv::Mat> m_sprites;

public:
    Rabbit(float x, float y, const std::vector<cv::Mat>& sprites);
    void respawn();
    float getVx() const { return m_vx; }
    float getVy() const { return m_vy; }

    void update(float wolf_center_x, float wolf_center_y, int score);
    void draw(cv::Mat& canvas);
    cv::Rect getHitbox() const;
};
