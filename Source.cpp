#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <iostream>

using namespace std;

const int WINDOW_WIDTH = 1500;
const int WINDOW_HEIGHT = 900;
const int GAME_DURATION = 60; // Game duration in seconds
const sf::Color COLOR_START = sf::Color::White;
const sf::Color COLOR_MIDDLE = sf::Color::Yellow;
const sf::Color COLOR_END = sf::Color::Red;
static  float speedIncrement = 50.0f; // Amount to increase speed

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

class Word {
public:
    Word(const std::string& text, const sf::Font& font, float initialSpeed)
        : text(text), speed(initialSpeed), initialSpeed(initialSpeed) {
        word.setFont(font);
        word.setString(text);
        word.setCharacterSize(24);
        word.setFillColor(sf::Color::White);
        word.setPosition(0, rand() % (WINDOW_HEIGHT - 50)); // Randomize the y-position
    }

    void update(float deltaTime) {
        word.move(speed * deltaTime, 0);
    }

    void render(sf::RenderWindow& window) {
        window.draw(word);
    }

    void updateColor() {
        float positionRatio = word.getPosition().x / WINDOW_WIDTH;
        if (positionRatio < 0.5f) {
            word.setFillColor(COLOR_START);
        }
        else if (positionRatio < 0.9f) {
            word.setFillColor(COLOR_MIDDLE);
        }
        else if (positionRatio < 1.1f) {
            word.setFillColor(COLOR_END);
        }
    }

    void updateSpeed(float elapsedTimeSinceLastIncrease) {
        // Increase speed over time for added challenge
        speed = initialSpeed + speedIncrement;
    }

    bool isOutOfBounds() const {
        return word.getPosition().x > WINDOW_WIDTH;
    }

    std::string getText() const {
        return text;
    }

private:
    sf::Text word;
    std::string text;
    float speed;
    float initialSpeed;
};


std::vector<Word> words;
sf::Clock gameClock;
sf::Clock wordClock;
float wordSpawnInterval = 2.0f; // Spawn a word every 2 seconds
sf::Font font;
sf::Text testText; // Text to test font change
int currentFontIndex = 0; // Index of current font
std::string userInput;
int score = 0;
int missedWords = 0;
GameState gameState = GameState::MENU;

sf::SoundBuffer successBuffer;
sf::Sound successSound;
sf::SoundBuffer missedBuffer;
sf::Sound missedSound;

std::vector<std::string> wordList = {
    "apple", "banana", "cherry", "date", "elderberry", "fig", "grape", "honeydew",
    "kiwi", "lemon", "mango", "nectarine", "orange", "papaya", "quince", "raspberry",
    "strawberry", "tangerine", "ugli", "vanilla", "watermelon", "xigua", "yam", "zucchini",
    "avocado", "blackberry", "blueberry", "cantaloupe", "coconut", "cranberry", "dragonfruit",
    "grapefruit", "guava", "jackfruit", "kumquat", "lime", "lychee", "mulberry", "olive",
    "peach", "pear", "pineapple", "plum", "pomegranate", "pumpkin", "rhubarb", "starfruit",
    "umbrella", "computer", "keyboard", "monitor", "mouse", "chair", "table", "pencil",
    "notebook", "phone", "television", "radio", "camera", "light", "sofa", "bed", "blanket",
    "pillow", "window", "door", "ceiling", "floor", "wall", "mirror", "painting", "clock",
    "book", "shelf", "cabinet", "drawer", "lamp", "fan", "heater", "air conditioner", "cup",
    "bottle", "plate", "spoon", "fork", "knife", "stove", "oven", "refrigerator", "microwave",
    "toaster", "blender", "dishwasher", "washing machine", "dryer", "iron", "vacuum"
};

void applySpeedIncrement(float speedIncrement) {
    for (auto& word : words) {
        word.updateSpeed(speedIncrement);
    }
}


void updateWords(float deltaTime) {
    static float timeSinceLastSpeedIncrease = 0.0f;
    static const float speedIncreaseInterval = 15.0f; // Increase speed every 15 seconds
   

    // Update words' position, color, and speed based on deltaTime
    for (auto& word : words) {
        word.update(deltaTime);
        word.updateColor();
    }

    // Remove out-of-bounds words
    words.erase(std::remove_if(words.begin(), words.end(), [](const Word& word) {
        if (word.isOutOfBounds()) {
            missedWords++;
            missedSound.play();
            return true;
        }
        return false;
        }), words.end());

    // Spawn new words
    if (wordClock.getElapsedTime().asSeconds() > wordSpawnInterval) {
        std::string randomWord = wordList[rand() % wordList.size()];
        Word newWord(randomWord, font, 100.0f); // Initial speed can be adjusted
        words.push_back(newWord); // Add new word to the vector
        wordClock.restart();

        // Apply current speed increment to the newly spawned word
        newWord.updateSpeed(timeSinceLastSpeedIncrease);
    }

    // Increase speed of words every speedIncreaseInterval seconds
    timeSinceLastSpeedIncrease += deltaTime;
    if (timeSinceLastSpeedIncrease >= 15.0f && timeSinceLastSpeedIncrease < 30.0f) {
        applySpeedIncrement(speedIncrement);
        speedIncrement = 100.0f;
    }
    else if (timeSinceLastSpeedIncrease >= 30.0f && timeSinceLastSpeedIncrease < 45.0f) {
        applySpeedIncrement(speedIncrement);
        speedIncrement = 150.0f;
    }
    else if (timeSinceLastSpeedIncrease >= 45.0f) {
        applySpeedIncrement(speedIncrement);
        speedIncrement = 200.0f;
    }

 
}



void handleInput(sf::Event event, sf::Text& userInputText) {
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b' && !userInput.empty()) {
            userInput.pop_back();
        }
        else if (event.text.unicode < 128 && event.text.unicode != '\b') {
            userInput += static_cast<char>(event.text.unicode);
        }

        // Update the user input text display
        userInputText.setString(userInput);

        for (auto it = words.begin(); it != words.end();) {
            if (userInput == it->getText()) {
                it = words.erase(it);
                userInput.clear();
                userInputText.setString(userInput); // Clear the displayed text
                score++;
                successSound.play(); // Play success sound effect
            }
            else {
                ++it;
            }
        }
    }
}

void displayMenu(sf::RenderWindow& window, sf::Text& startGameText, sf::Text& quitGameText, sf::Text& changeFontText) {
    window.clear();

    window.draw(startGameText);
    window.draw(quitGameText);
    window.draw(changeFontText);
    window.draw(testText);

    window.display();
}

void displayGameOver(sf::RenderWindow& window, sf::Text& gameOverText, sf::Text& scoreText, sf::Text& missedText) {
    window.clear();

    window.draw(gameOverText);
    window.draw(scoreText);
    window.draw(missedText);

    window.display();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Monkey Typer");

    // Load all fonts
    sf::Font font1, font2, font3, font4;
    if (!font1.loadFromFile("C:/Users/hamza/Downloads/uniqloves-bold-font/UniqlovesBoldBold-4Bm8K.ttf") || !font2.loadFromFile("C:/Users/hamza/Downloads/thirteen-pixel-fonts/ThirteenPixelFontsRegular-wjR3.ttf") || !font3.loadFromFile("C:/Users/hamza/Downloads/highup-italic-font/HighupItalicPersonalUseBoldItalic-vmqnM.ttf") || !font4.loadFromFile("C:/Users/hamza/Downloads/winter-style-font/WinterStyle-rgPqL.ttf")) {
        // Handle error
        return -1;
    }

    // Set initial font
    font = font1;

    // Load sound buffers
    if (!successBuffer.loadFromFile("C:/Users/hamza/Downloads/sounds/success-1-6297.wav") || !missedBuffer.loadFromFile("C:/Users/hamza/Downloads/sounds/fx-failed-spell-92292.wav")) {
        // Handle error
        return -1;
    }

    // Set sound buffers
    successSound.setBuffer(successBuffer);
    missedSound.setBuffer(missedBuffer);

    srand(static_cast<unsigned int>(time(nullptr))); // Seed for random word selection

    // Text object for displaying user input
    sf::Text userInputText;
    userInputText.setFont(font);
    userInputText.setCharacterSize(24);
    userInputText.setFillColor(sf::Color::Green);
    userInputText.setPosition(10, WINDOW_HEIGHT - 40); // Position at the bottom

    // Text object for displaying countdown timer
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::Red);
    timerText.setPosition(WINDOW_WIDTH - 130, 10); // Position at the top right

    // Text objects for menu
    sf::Text startGameText;
    startGameText.setFont(font);
    startGameText.setCharacterSize(30);
    startGameText.setFillColor(sf::Color::White);
    startGameText.setString("Start Game");
    startGameText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);

    sf::Text quitGameText;
    quitGameText.setFont(font);
    quitGameText.setCharacterSize(30);
    quitGameText.setFillColor(sf::Color::White);
    quitGameText.setString("Quit");
    quitGameText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 20);

    sf::Text changeFontText;
    changeFontText.setFont(font);
    changeFontText.setCharacterSize(24);
    changeFontText.setFillColor(sf::Color::White);
    changeFontText.setString("Change Font");
    changeFontText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 100);

    // Text objects for game over screen
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("Game Over!");
    gameOverText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 100);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);

    sf::Text missedText;
    missedText.setFont(font);
    missedText.setCharacterSize(30);
    missedText.setFillColor(sf::Color::White);
    missedText.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);

    sf::Clock gameDurationClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (gameState == GameState::MENU) {
                if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (startGameText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        gameState = GameState::PLAYING;
                        gameDurationClock.restart();
                        gameClock.restart();
                    }
                    else if (quitGameText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        window.close();
                    }
                    else if (changeFontText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        // Cycle through fonts for the test text
                        currentFontIndex = (currentFontIndex + 1) % 4;
                        switch (currentFontIndex) {
                        case 0:
                            font = font1;
                            break;
                        case 1:
                            font = font2;
                            break;
                        case 2:
                            font = font3;
                            break;
                        case 3:
                            font = font4;
                            break;
                        default:
                            break;
                        }
                        testText.setFont(font);
                        startGameText.setFont(font);
                        quitGameText.setFont(font);
                        changeFontText.setFont(font);
                    }
                }
            }
            else if (gameState == GameState::PLAYING) {
                handleInput(event, userInputText);
            }
        }

        if (gameState == GameState::MENU) {
            displayMenu(window, startGameText, quitGameText, changeFontText);
        }
        else if (gameState == GameState::PLAYING) {
            window.clear();

            float deltaTime = gameClock.restart().asSeconds();
            updateWords(deltaTime);

            for (auto& word : words) {
                word.render(window);
            }

            // Update and render the countdown timer
            int remainingTime = GAME_DURATION - static_cast<int>(gameDurationClock.getElapsedTime().asSeconds());
            timerText.setString("Time: " + std::to_string(remainingTime));
            window.draw(timerText);

            // Render the user input text
            window.draw(userInputText);

            window.display();

            // Check if game time is over
            if (remainingTime <= 0) {
                gameState = GameState::GAME_OVER;
                window.clear();

                scoreText.setString("Score: " + std::to_string(score));
                missedText.setString("Missed Words: " + std::to_string(missedWords));

                displayGameOver(window, gameOverText, scoreText, missedText);
            }
        }
    }

    return 0;
}

