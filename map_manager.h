#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "common.h"

class MapManager {
private:
    std::vector<Tree> m_trees;
    std::vector<cv::Mat> m_tree_sprites; // ИСПРАВЛЕНИЕ: std::vector вместо cv::Mat
    float m_wolf_diameter;


public:
    MapManager(const cv::Mat& t1, const cv::Mat& t2, const cv::Mat& t3, float wolf_size);
    
    void generateMap(int score); // Изменим внутреннюю логику
    bool addSingleTree();        // Новый приватный или публичный метод
    void checkWolfCollisions(class Wolf& wolf) const;
    void drawTrees(cv::Mat& canvas) const;
    const std::vector<Tree>& getTrees() const { return m_trees; }
};
