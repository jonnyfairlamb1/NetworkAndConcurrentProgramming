/*
  Snake ICA start project using SFML
    SFML documentation: http://www.sfml-dev.org/documentation/2.4.2/classes.php
  Windows build
    To build on Windows you need to have the correct version of CodeBlocks
      Download from here: http://www.codeblocks.org/downloads/26
      Must be the download titled "codeblocks-16.01mingw-setup.exe"
*/

#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <map>
#include <cstdlib>

// SFML header file for graphics, there are also ones for Audio, Window, System and Network
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <boost/algorithm/string.hpp>

struct playerData
{
    std::string playerNickname = " ";
    bool registered = false;
    int posX = 0;
    int posY = 0;
    int speed = 5;

    bool update = false;
    bool sendChallenge = false;
    bool challengeHasBeenSent = false;
    bool challengeHasBeenRecieved = false;
    std::string challengeFrom = " ";
    bool inSession = false;
    bool sentChoice = false;
    bool receivedChoice = false;
    int enemyChoice = 0;
    int playerChoice = 0;
};


    std::vector<playerData> Players;
    playerData mainPlayer;
    sf::TcpSocket tcp_socket;

void tcpSend(std::string send)
{
    if (tcp_socket.send(send.c_str(), send.size()) != sf::Socket::Done)
    {
        std::cerr << "sending failed" << std::endl;
    }
}

void recv_loop(sf::TcpSocket& socket)
{
    char buffer_in[1024];
    std::size_t received;
    bool exists = false;

    while(true){

        if (socket.receive(buffer_in, 88, received) != sf::Socket::Done)
        {
        }
        std::cout << buffer_in << std::endl;

        //set positions
        std::vector<std::string> message;
        std::string stringin = buffer_in;
        boost::split(message, stringin, boost::is_any_of(" "));

        if(message[0] == "chal")
        {
            if(!mainPlayer.inSession)
            {
                mainPlayer.challengeHasBeenRecieved = true;
            }
        }
        else if(message[0] == "choice")
        {
            mainPlayer.enemyChoice = std::stoi(message[1]);
            mainPlayer.receivedChoice = true;
        }
        else
        {
            for(auto& p : Players){

                if(p.playerNickname == message[0])
                {
                    p.posX = std::stoi(message[1]);
                    p.posY = std::stoi(message[2]);
                    exists = true;
                }
            }

            if(!exists)
            {

                playerData newplayer;
                newplayer.playerNickname = message[0];
                newplayer.posX = std::stoi(message[1]);
                newplayer.posY = std::stoi(message[2]);
                Players.push_back(newplayer);
                std::cout << "create new player" << std::endl;
            }
            exists = false;
        }


    }
}

struct client_t
{
    sf::TcpSocket & tcp_socket;
    client_t(sf::TcpSocket & _tcp_socket):
        tcp_socket(_tcp_socket){}
    void operator()()
    {
        recv_loop(tcp_socket);
    }
};

void RockPaperScissors()
{
    if(!mainPlayer.sentChoice)
    {
        // TCP send
        if(mainPlayer.playerChoice < 4 && mainPlayer.playerChoice > 0)
        {
            std::string send = "choice--" + mainPlayer.challengeFrom + "--" + std::to_string(mainPlayer.playerChoice);

            tcpSend(send);

            mainPlayer.sentChoice = true;
        }
        else
        {
            std::cout << "Choose between rock(1), paper(2), scissors(3)" << std::endl;
            std::cin >> mainPlayer.playerChoice;
        }

    }
    if(mainPlayer.sentChoice && mainPlayer.receivedChoice)
    {

        //compare
        if(mainPlayer.playerChoice == 1 && mainPlayer.enemyChoice == 2)
        {
            //enemy Win

            std::cout << "Your Enemy Won!"  << std::endl;
            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == 1 && mainPlayer.enemyChoice == 3)
        {
            std::cout << "You Won!"  << std::endl;

            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == 2 && mainPlayer.enemyChoice == 1)
        {
            std::cout << "You Won!"  << std::endl;
            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == 2 && mainPlayer.enemyChoice == 3)
        {
            //enemy win
            std::cout << "Your Enemy Won!"  << std::endl;
            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == 3 && mainPlayer.enemyChoice == 1)
        {
            //enemy win
            std::cout << "Your Enemy Won!"  << std::endl;
            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == 3 && mainPlayer.enemyChoice == 2)
        {
            //player win
            std::cout << "You Won!"  << std::endl;
            mainPlayer.inSession = false;
        }
        else if(mainPlayer.playerChoice == mainPlayer.enemyChoice )
        {
            std::cout << "Its a Tie!"  << std::endl;
            //tie
            mainPlayer.inSession = false;

        }
    }

}

void registerPlayer()
{
     std::cout << "Please Enter Your Player Nickname" << std::endl;
    std::cin >> mainPlayer.playerNickname;

    if(mainPlayer.playerNickname != " ")
    {
        // TCP send
        std::string send = "reg--" + mainPlayer.playerNickname;
        tcpSend(send);

        mainPlayer.registered = true;
        Players.push_back(mainPlayer);

    }

}

void unregisterPlayer()
{
     std::cout << "Please Enter Your Name" << std::endl;
    std::cin >> mainPlayer.playerNickname;

    if(mainPlayer.playerNickname != " ")
    {
        // TCP send
        std::string send = "unreg--" + mainPlayer.playerNickname;
        tcpSend(send);
    }

}

void setDefaults()
{
    mainPlayer.sendChallenge = false;
    mainPlayer.challengeHasBeenSent = false;
    mainPlayer.challengeHasBeenRecieved = false;
    mainPlayer.challengeFrom = " ";
    mainPlayer.inSession = false;
    mainPlayer.sentChoice = false;
    mainPlayer.receivedChoice = false;
    mainPlayer.playerChoice = 0;
    mainPlayer.enemyChoice = 0;
}

int main()
{

    sf::Socket::Status status = tcp_socket.connect("127.0.0.1", 13000);
    if (status != sf::Socket::Done)
    {
        std::cerr << "connect failed\n";
    }

    //registers player
    registerPlayer();

    //start receive loop thread
    client_t client(tcp_socket);
    std::thread t(client);

    // Create an instance of the SFML RenderWindow type which represents the display
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Rock Paper Scissors Q5058597",sf::Style::Default,settings);

    // Set the shape's fill colour
    sf::CircleShape circle{25.f};

    // We can still output to the console window
    std::cout << "Starting" << std::endl;

    // Main loop that continues until we call Close()
    while (window.isOpen() && mainPlayer.registered)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
          switch(event.type)
          {
            case sf::Event::Closed:
              window.close();
            break;
            default:
              break;
          }
        }

        //Keyboard inputs
        if(!mainPlayer.inSession)
            {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                mainPlayer.posX -= mainPlayer.speed;
                mainPlayer.update = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                mainPlayer.posX += mainPlayer.speed;
                mainPlayer.update = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                mainPlayer.posY -= mainPlayer.speed;
                mainPlayer.update = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                mainPlayer.posY += mainPlayer.speed;
                mainPlayer.update = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::C))
            {
                if(!mainPlayer.challengeHasBeenRecieved)
                {
                    mainPlayer.challengeHasBeenSent = true;
                }
                else
                {
                    mainPlayer.inSession = true;
                }
            }
        }

        if(mainPlayer.update == true)
        {
            std::string send = "_";

            //Send position and name data to the server which sends it out to every other client
            //sends position and name to server which in turn sends it to every client connected but yourself
            send = "pos--" + mainPlayer.playerNickname + "--" + std::to_string(mainPlayer.posX) + "--" + std::to_string(mainPlayer.posY);
            tcpSend(send);
            mainPlayer.update = false;
        }

        //sending challenge to other player
        for(auto& p : Players)
        {
            if(p.playerNickname != mainPlayer.playerNickname){
            //if other player is in range
                if (mainPlayer.posX < p.posX + 100 && mainPlayer.posX > p.posX - 50 &&
                    mainPlayer.posY < p.posY + 100 && mainPlayer.posY > p.posY - 50)
                {
                    if(mainPlayer.challengeHasBeenRecieved)
                    {
                        mainPlayer.challengeFrom = p.playerNickname;
                    }
                    if(mainPlayer.sendChallenge)
                    {
                        if(!mainPlayer.challengeHasBeenSent)
                        {
                            std::string send = "_";
                            send = "chal--" + mainPlayer.playerNickname + "--" + p.playerNickname;
                            tcpSend(send);
                        }
                        mainPlayer.sendChallenge = false;
                        mainPlayer.challengeHasBeenSent = true;
                    }
                }
                else
                {
                    mainPlayer.sendChallenge = false;
                    mainPlayer.challengeHasBeenSent = false;
                }
            }
        }
        if(mainPlayer.challengeHasBeenRecieved && mainPlayer.sendChallenge)
        {
            mainPlayer.inSession = true;
        }

        //Send Choice - RPS game
        if(mainPlayer.inSession)
        {
            RockPaperScissors();
        }
        else
        {
            setDefaults();
        }

        // We must clear the window each time round the loop
        window.clear();

        for(auto& p : Players){
            if(p.playerNickname != mainPlayer.playerNickname){
                circle.setPosition(p.posX, p.posY);
                circle.setFillColor(sf::Color::Red);
                window.draw(circle);

            }else{
                circle.setPosition(mainPlayer.posX, mainPlayer.posY);
                circle.setFillColor(sf::Color::Blue);
                window.draw(circle);
            }

        }
        window.display();
    }


    std::cout << "Finished" << std::endl;
    unregisterPlayer();
    t.join();
    return 0;
}


