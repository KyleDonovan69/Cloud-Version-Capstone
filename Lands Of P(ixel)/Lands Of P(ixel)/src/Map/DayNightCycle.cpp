#include "DayNightCycle.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

DayNightCycle::DayNightCycle()
    : m_timeOfDay(0.25f)  // start at dawn (6am)
    , m_currentDay(1)
    , m_dayLength(120.0f) // how many seconds for full day cycle
{
    // overlay covers entire screen
    m_overlay.setSize(sf::Vector2f(10.f, 10.f)); // now set to scale with the function
    m_overlay.setFillColor(sf::Color(0, 0, 50, 0)); // starts transparent

    if (!m_font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "problem loading font for day/night cycle" << std::endl;
    }

    m_timeDisplay.setFont(m_font);
    m_timeDisplay.setCharacterSize(25U);
    m_timeDisplay.setFillColor(sf::Color::White);
    m_timeDisplay.setOutlineColor(sf::Color::Black);
    m_timeDisplay.setOutlineThickness(2.0f);
    m_timeDisplay.setPosition(sf::Vector2f{ 20.f, 60.f }); // below controls text
}

void DayNightCycle::update(sf::Time t_deltaTime)
{
    // advances time
    float timeIncrement = t_deltaTime.asSeconds() / m_dayLength;
    m_timeOfDay += timeIncrement;

    // new day when we pass 1.0
    if (m_timeOfDay >= 1.0f)
    {
        m_timeOfDay -= 1.0f;
        m_currentDay++;
        std::cout << "Day " << m_currentDay << " has begun!" << std::endl;
    }

    updateOverlay();
    m_timeDisplay.setString(getTimeString());
}

void DayNightCycle::draw(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    m_overlay.setSize(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    m_overlay.setPosition(sf::Vector2f(0, 0));
    // draw the overlay on top of everything
    window.draw(m_overlay);
}

bool DayNightCycle::isNight() const
{
    // night is from 0.75 (9pm) to 0.25 (6am)
    return m_timeOfDay >= 0.75f || m_timeOfDay < 0.25f;
}

int DayNightCycle::getCurrentDay() const
{
    return m_currentDay;
}

float DayNightCycle::getTimeOfDay() const
{
    return m_timeOfDay;
}

void DayNightCycle::resetTime()
{
    m_timeOfDay = 0.25f;
    m_currentDay = 1;
}

void DayNightCycle::setDayLengthMinutes(float minutes)
{
    // minutes to seconds
    m_dayLength = minutes * 60.0f;
    std::cout << "Day/night cycle set to " << minutes << " minutes (" << m_dayLength << " seconds)" << std::endl;
}

void DayNightCycle::updateOverlay()
{
    sf::Color overlayColor = calculateSkyColor();
    m_overlay.setFillColor(overlayColor);
}

sf::Color DayNightCycle::calculateSkyColor() const
{
    if (m_timeOfDay < 0.25f)
    {
        // night to dawn (midnight to 6am)
        float t = m_timeOfDay / 0.25f; // 0 to 1
        int alpha = static_cast<int>(150 - (150 * t)); // fade out darkness
        return sf::Color(0, 0, 50, alpha);
    }
    else if (m_timeOfDay < 0.5f)
    {
        // dawn to noon (6am to 12pm) - fully bright, changing later for mornings
        return sf::Color(0, 0, 0, 0); // no overlay
    }
    else if (m_timeOfDay < 0.75f)
    {
        // noon to dusk (12pm to 9pm) - still bright
        return sf::Color(0, 0, 0, 0);
    }
    else
    {
        // dusk to night (9pm to midnight)
        float t = (m_timeOfDay - 0.75f) / 0.25f; // 0 to 1
        int alpha = static_cast<int>(150 * t); // fade in darkness
        return sf::Color(0, 0, 50, alpha);
    }
}

std::string DayNightCycle::getTimeString() const
{
    // convert 0-1 to 24 hour time
    int totalMinutes = static_cast<int>(m_timeOfDay * 24 * 60);
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    // convert to 12 hour format for display
    std::string period = (hours >= 12) ? "PM" : "AM";
    int displayHours = hours % 12;
    if (displayHours == 0) displayHours = 12;

    std::stringstream ss;
    ss << "Day " << m_currentDay << " - " << displayHours << ":" << std::setfill('0') << std::setw(2) << minutes << " " << period;
    return ss.str();
}