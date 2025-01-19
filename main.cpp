#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <conio.h>

#if defined(__MINGW32__) || defined(_MSC_VER)
#include <windows.h>
#endif

class Console
{
public:
    // Kody klawiszy sterujących kursorem i klawiszy specjalnych
    enum KEY_CODES
    {
#ifdef __BORLANDC__
        KEY_BLANK = 0x0000,
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
        KEY_BLANK = 0x00e0,
#endif
        KEY_UP = 0x4800,
        KEY_DOWN = 0x5000,
        KEY_LEFT = 0x4b00,
        KEY_RIGHT = 0x4d00,
        KEY_ESC = 0x001b
    };
    static int getKey();
    static void clearScreen();
    static void writeCharXY(int x, int y, char c);
    static void writeStrXY(int x, int y, const char s[]);
    static void writeIntXY(int x, int y, int i);
    static void writeDoubleXY(int x, int y, double d);
    static void cursorOff();
    static void cursorOn();
};

class GameObject
{
public:
    GameObject(int x = 0, int y = 0, int shape = ' ', int power = 0)
            : x(x), y(y), shape(shape), isVisible(false), power(power)
    {
    }
    virtual void show()
    {
        Console::writeCharXY(x, y, shape);
        isVisible = true;
    }
    virtual void hide()
    {
        Console::writeCharXY(x, y, ' ');
        isVisible = false;
    }
    bool hasTheSamePositionAs(const GameObject & otherObject)
    {
        return (x == otherObject.x and y == otherObject.y);
    }
    virtual void update()
    {
    }

    int x;
    int y;
    int shape;
    bool isVisible;
    int power;
};


class TimedObject : public GameObject
{
public:
    TimedObject(int x = 0, int y = 0, int shape = ' ', int power = 0, long sleepTime = 1000)
            : GameObject(x, y, shape, power), sleepTime(sleepTime), active(false), lastActivationTime(0)
    {
    }
    void show()
    {
        GameObject::show();
        activate();
    }
    void activate()
    {
        active = true;
        updateLastActivationTime();
    }
    void deactivation()
    {
        hide();
        active = false;
    }
    bool isActive()
    {
        return active;
    }

    bool activationTimeEllapsed()
    {

        long t = time(0);
        return t - lastActivationTime > sleepTime;
    }

    void update()
    {
        if(isActive())
        {
            if(activationTimeEllapsed())
                deactivation();
        }
    }
private:
    void updateLastActivationTime()
    {
        lastActivationTime = time(0);
    }
    long int lastActivationTime;

    long sleepTime;
    bool active;
};

class MovingPoint : public GameObject
{
public:
    MovingPoint() : GameObject()
    {
    }
    MovingPoint(int x, int y, int shape, int power) : GameObject(x, y, shape, power)
    {
    }
    void moveUp()
    {
        hide();
        if(y > 1)
            --y;
        show();
    }
    void moveDown()
    {
        hide();
        if(y < 24)
            ++y;
        show();
    }
    void moveLeft()
    {
        hide();
        if(x > 1)
            --x;
        show();
    }
    void moveRight()
    {
        hide();
        if(x < 80)
            ++x;
        show();
    }
};

class Player : public MovingPoint
{
public:
    Player() : MovingPoint(), health(100)
    {
    }
    Player(int x, int y, int shape) : MovingPoint(x, y, shape, 0), health(100)
    {
    }
    int health;
};

class Maze
{
public:
    enum MAZE_PARAMS
    {
        NUM_OF_ROWS = 5,
        NUM_OF_COLS = 10
    };
    void show();
    bool isWallOnXY(int x, int y);
private:
    static char walls[NUM_OF_ROWS][NUM_OF_COLS + 1];
    bool validPosition(int x, int y);
};
char Maze::walls[NUM_OF_ROWS][NUM_OF_COLS + 1] =
        {
                "##########",
                "# ##    ##",
                "# ## ## ##",
                "#    ## ##",
                "## #######",
        };
void Maze::show()
{
    for(int row = 0; row < NUM_OF_ROWS; ++row)
        Console::writeStrXY(1, row + 1, walls[row]);
}
bool Maze::isWallOnXY(int x, int y)
{
    if(validPosition(x, y))
        return walls[y - 1][x - 1] == '#';
    else
        return false;
}
bool Maze::validPosition(int x, int y)
{
    if(!(x > 0 && x <= NUM_OF_COLS))
        return false;
    if(!(y > 0 && y <= NUM_OF_ROWS))
        return false;
    return true;
}

const int NUM_OF_BOMBS = 6;
class TheGame
{
public:

    TheGame() : player(10, 12, '*'), maze(), dinner(6, 2, '%', 10)
    {
        Console::cursorOff();
        Console::writeStrXY(15, 12, "Nacisnij dowony klawisz by rozpoczac gre");
        Console::getKey();
        Console::clearScreen();
        gameObjects[0] = new GameObject(2, 2, '@', -10);
        gameObjects[1] = new GameObject(4, 4, '$', -5);
        gameObjects[2] = new GameObject(8, 4, '?', -100);
        gameObjects[3] = new TimedObject(10, 15, 'W', -10, 2);
        gameObjects[4] = new TimedObject(12, 15, 'M', -5, 3);
        gameObjects[5] = new TimedObject(14, 15, 'H', -100, 5);
    }
    void showBombs()
    {
        for(int i = 0; i < NUM_OF_BOMBS; ++i)
            gameObjects[i]->show();
    }
    ~TheGame()
    {
        Console::clearScreen();
        Console::writeStrXY(15, 12, "Nacisnij dowony klawisz by zakonczyc gre");
        Console::getKey();
        Console::cursorOn();
        for(int i = 0; i < NUM_OF_BOMBS; ++i)
            delete gameObjects[i];
    }

    void run()
    {
        int key;
        maze.show();
        player.show();
        showBombs();
        dinner.show();
        do
        {
            if(!kbhit())
                backgroundProcess();
            else
            {
                key = Console::getKey();
                if(!keyboardPreProcessed(key))
                {
                    processKeyboard(key);
                    postProcessKeyboard(key);
                }
            }
        }
        while(key != Console::KEY_ESC);
    }

private:

    void postProcessKeyboard(int key)
    {
        for(int i = 0; i < NUM_OF_BOMBS; ++i)
            if(gameObjects[i]->isVisible)
                if(player.hasTheSamePositionAs(*gameObjects[i]))
                {
                    player.health += gameObjects[i]->power;
                    gameObjects[i]->hide();
                    player.show();
                }

        if(dinner.isVisible)
            if(player.x == dinner.x && player.y == dinner.y)
            {
                player.health += dinner.power;
                dinner.hide();
                player.show();
            }
    }
    void backgroundProcess()
    {
        char strBuffer[80];
        sprintf(strBuffer, "Pozycja %2d %2d Sily zyciowe: %3d", player.x, player.y, player.health);
        Console::writeStrXY(2, 25, strBuffer);
        showClock();
        updateObjects();
    }

    void updateObjects()
    {
        for(int i = 0; i < NUM_OF_BOMBS; ++i)
            gameObjects[i]->update();
    }
    void showClock()
    {
        time_t t;
        time(&t);
        // Pierwszy sposób
        //Console::writeStrXY(30, 25, ctime(&t));
        // Drugi sposób
        tm * timeInfo = localtime(&t);
        char buffer[80];
        sprintf(buffer, "%02d:%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        Console::writeStrXY(70, 25, buffer);
    }
    void processKeyboard(int key)
    {
        movePoint(key);
    }

    void movePoint(int key)
    {
        switch(key)
        {
            case Console::KEY_UP:
                player.moveUp();
                break;
            case Console::KEY_DOWN:
                player.moveDown();
                break;
            case Console::KEY_LEFT:
                player.moveLeft();
                break;
            case Console::KEY_RIGHT:
                player.moveRight();
                break;
        }
    }

    bool keyboardPreProcessed(int key)
    {
        int newX = player.x;
        int newY = player.y;

        switch(key)
        {
            case Console::KEY_UP:
                --newY;
                break;
            case Console::KEY_DOWN:
                ++newY;
                break;
            case Console::KEY_LEFT:
                --newX;
                break;
            case Console::KEY_RIGHT:
                ++newX;
                break;
        }
        return maze.isWallOnXY(newX, newY);
    }
    Player player;
    Maze maze;
    GameObject * gameObjects[NUM_OF_BOMBS];
    GameObject dinner;
};

int main()
{
    TheGame game;
    game.run();

    return EXIT_SUCCESS;
}

//https://pastebin.com/x0EQtVGg

// Funkcje obsługi konsoli, sprawa techniczna, w sensie merytorycznym nieistotna
int Console::getKey()
{
    int key = _getch();
    return (key == KEY_BLANK) ? _getch() << 8 : key;
}

void Console::clearScreen()
{
#ifdef __BORLANDC__
    clrscr();
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
    COORD leftTop = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    int numOfCells = 80 * 25;
    DWORD writtenItems;
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo))
        numOfCells = consoleInfo.dwSize.X * consoleInfo.dwSize.Y;

    FillConsoleOutputAttribute(consoleHandle, 0xf,
                               numOfCells, leftTop, &writtenItems);
    FillConsoleOutputCharacter(consoleHandle, ' ',
                               numOfCells, leftTop, &writtenItems);
#else
#error "Nieobslugiwana platforma"
#endif
}

void Console::writeCharXY(int x, int y, char c) {
#ifdef __BORLANDC__
    gotoxy(x, y);
    putch(c);
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
    COORD cursorPos;
    DWORD written;

    cursorPos.X = x - 1;
    cursorPos.Y = y - 1;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), &c, 1, &written, 0);
#else
#error "Nieobslugiwana platforma"
#endif
}

void Console::writeStrXY(int x, int y, const char s[])
{
#ifdef __BORLANDC__
    gotoxy(x, y);
    cputs(s);
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
    COORD cursorPos;
    DWORD written;

    cursorPos.X = x - 1;
    cursorPos.Y = y - 1;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), s, strlen(s), &written, 0);
#else
#error "Nieobslugiwana platforma"
#endif
}

void Console::writeIntXY(int x, int y, int i)
{
    char s[80];
#if defined(_MSC_VER)
    sprintf_s(s, 80, "%d", i);
#else
    sprintf(s, "%d", i);
#endif
    writeStrXY(x, y, s);
}

void Console::writeDoubleXY(int x, int y, double d)
{
    char s[80];
#if defined(_MSC_VER)
    sprintf_s(s, 80, "%g", d);
#else
    sprintf(s, "%g", d);
#endif
    writeStrXY(x, y, s);
}

void Console::cursorOff()
{
#ifdef __BORLANDC__
    _setcursortype(_NOCURSOR);
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
#else
#error "Nieobslugiwana platforma"
#endif
}

void Console::cursorOn()
{
#ifdef __BORLANDC__
    _setcursortype(_NORMALCURSOR);
#elif (defined(__GNUC__) && defined(__MINGW32__)) || defined(_MSC_VER)
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
#else
#error "Nieobslugiwana platforma"
#endif
}