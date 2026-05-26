#include "map_manager.h"
#include "wolf.h"
#include <cmath>
#include <cstdlib>

MapManager::MapManager(const cv::Mat& t1, const cv::Mat& t2, const cv::Mat& t3, float wolf_size) {
    m_tree_sprites.push_back(t1);
    m_tree_sprites.push_back(t2);
    m_tree_sprites.push_back(t3);
    m_wolf_diameter = wolf_size;
}

// Новый метод для умной генерации одной елки с проверкой пролезания
bool MapManager::addSingleTree() {
    float base_radius = 45.0f;
    int attempts = 0;
    
    while (attempts < 300) {
        attempts++;
        int tx = rand() % (GAME_W - 200) + 100;
        int ty = rand() % (GAME_H - 250) + 100;
        float cx = tx + 50.0f;
        float cy = ty + 50.0f;

        // Защита центра экрана (где спавнится Волк)
        float dist_to_center = std::sqrt((cx - GAME_W/2)*(cx - GAME_W/2) + (cy - GAME_H/2)*(cy - GAME_H/2));
        if (dist_to_center < 160.0f) continue;

        bool valid_position = true;

        // Проверяем расстояние до ВСЕХ уже существующих елок на карте
        for (const auto& existing_tree : m_trees) {
            float ex_cx = existing_tree.x + 50.0f;
            float ex_cy = existing_tree.y + 50.0f;
            float dist = std::sqrt((cx - ex_cx)*(cx - ex_cx) + (cy - ex_cy)*(cy - ex_cy));

            // Проверка пролезания Волка
            float min_allowed_distance = base_radius + existing_tree.radius + m_wolf_diameter + 15.0f;
            if (dist < min_allowed_distance) {
                valid_position = false;
                break;
            }
        }

        if (valid_position) {
            int sprite_idx = rand() % 3;
            m_trees.push_back({tx, ty, base_radius, m_tree_sprites[sprite_idx]});
            return true; // Елка успешно посажена!
        }
    }
    return false; // Не нашли места
}

void MapManager::generateMap(int score) {
    // Если игра только началась (score == 0) — сажаем 3 стартовые елки
    if (score == 0) {
        m_trees.clear();
        for (int i = 0; i < 3; ++i) {
            addSingleTree();
        }
        return;
    }

    // ТВОЕ ТРЕБОВАНИЕ: Добавляем ровно ОДНУ елку спустя каждые 3 уровня (3, 6, 9, 12...)
    if (score % 3 == 0) {
        // Защита от перенасыщения леса (максимум 15 деревьев)
        if (m_trees.size() < 15) {
            std::cout << "[MAP] Новый уровень сложности! Добавляю елку на карту. Всего: " << m_trees.size() + 1 << std::endl;
            addSingleTree();
        }
    }
}

void MapManager::checkWolfCollisions(Wolf& wolf) const {
    for (const auto& tree : m_trees) {
        wolf.handleCollisionWithBush(tree.x + 50, tree.y + 50, tree.radius);
    }
}

void MapManager::drawTrees(cv::Mat& canvas) const {
    for (const auto& tree : m_trees) {
        drawSprite(canvas, tree.sprite, tree.x, tree.y);
    }
}
