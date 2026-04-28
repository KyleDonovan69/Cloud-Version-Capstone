#pragma once
#include <SFML/Graphics.hpp>

class DayNightCycle
{
public:
    DayNightCycle();

    void update(sf::Time t_deltaTime);
    void draw(sf::RenderWindow& window);

    bool isNight() const;
    int getCurrentDay() const;
    float getTimeOfDay() const; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    void resetTime();
    void setDayLengthMinutes(float minutes); // Set day length in real-time minutes

private:
    float m_timeOfDay; // 0.0 to 1.0 based on t.o.d
    int m_currentDay;
    float m_dayLength; // how many seconds for full day/night cycle

    sf::RectangleShape m_overlay; // darkens screen at night
    sf::Font m_font;
    sf::Text m_timeDisplay{m_font};

    void updateOverlay();
    sf::Color calculateSkyColor() const;
    std::string getTimeString() const;
};