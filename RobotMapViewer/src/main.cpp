#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#define LOG(x) std::cout << x << std::endl

sf::Color grey = sf::Color(128, 128, 128);
char grid[500][500];
int sonarAngles[] = { 90, 50, 30, 10, -10, -30, -50, -90, -90, -130, -150, -170, 170, 150, 130, 90 };
std::pair<int, int> playerPos = { -1, -1 };

std::string getContents(const std::string& filePath)
{
	// Create file object and set exceptions
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// open files
		file.open(filePath);
		std::stringstream fileStream;
		// read file's buffer contents into streams
		fileStream << file.rdbuf();
		// close file handlers
		file.close();
		// convert stream into string
		return fileStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		LOG("Couldn't read file.");
	}
	return std::string();
}

std::string getLineFromString(const std::string& stringContents, const uint32_t lineIndex)
{
	if (stringContents != "")
	{
		std::istringstream iss(stringContents);
		uint32_t count = 0;

		for (std::string line; std::getline(iss, line); )
		{
			if (count == lineIndex)
				return line;

			count++;
		}
	}
	return "";
}

std::vector<int> getElementsInLine(std::string line)
{
	std::vector<int> elements;
	elements.resize(3);

	for (int i = 0; i < 3; i++)
	{
		int index = line.find(',');
		std::istringstream(line.substr(0, index)) >> elements[i];
		line = line.substr(index+1, std::string::npos);
	}
	return elements;
}

std::vector<float> getElementsInLineF(std::string line)
{
	std::vector<float> elements;
	elements.resize(3);

	for (int i = 0; i < 3; i++)
	{
		int index = line.find(',');
		std::istringstream(line.substr(0, index)) >> elements[i];
		line = line.substr(index + 1, std::string::npos);
	}
	return elements;
}

void createImage(sf::Image& image)
{
	image.create(500, 500, grey);

	// Go through the empty grid and initialise with our uncertain colour
	for (int i = 0; i < image.getSize().y; i++)
	{
		for (int j = 0; j < image.getSize().x; j++)
		{
			grid[j][i] = 0;
		}
	}
}

void editImage(sf::Image& image)
{
	// Go through each element in the grid and map the element onto the image
	for (int i = 0; i < image.getSize().y; i++)
	{
		for (int j = 0; j < image.getSize().x; j++)
		{
			sf::Color colour = sf::Color::Green;
			if (grid[i][j] == 0) colour = grey;
			else if (grid[i][j] == 1) colour = sf::Color::White;
			else if (grid[i][j] == 2) colour = sf::Color::Black;
			else if (grid[i][j] == 3) colour = sf::Color::Red;
			image.setPixel(i, j, colour);
		}
	}
}

void updateGrid(std::string filePath)
{
	std::string fileContents = getContents(filePath);

	// CoppeliaSim overwrites the file and there is a brief time when this program will open an empty file, in those particular events, just exit out this function
	if (fileContents.empty() || fileContents == "") return;

	// Delete all previous robot positions
	if(playerPos.first != -1) grid[playerPos.first][playerPos.second] = 1;

	// The robot prints its position and the sensor hits in the cell coordinates
	std::string robotData = getLineFromString(fileContents, 0);
	auto robotPos = getElementsInLine(robotData);
	grid[robotPos[0]][robotPos[1]] = robotPos[2];
	playerPos = { robotPos[0], robotPos[1] };

	// Now we can go through each sensor coordinate and set the value of the cell to 2
	for (int i = 1; i < 17; i++)
	{
		std::string sensorLine = getLineFromString(fileContents, i);
		auto sensorInfo = getElementsInLine(sensorLine);
		if(sensorInfo[0] != -1 && sensorInfo[1] != -1)
			grid[sensorInfo[0]][sensorInfo[1]] = sensorInfo[2];

	}
}

void updateRobotData(std::string filePath, std::vector<sf::Text>& texts)
{
	std::string fileContents = getContents(filePath);

	// CoppeliaSim overwrites the file and there is a brief time when this program will open an empty file, in those particular events, just exit out this function
	if (fileContents.empty() || fileContents == "") return;

	std::string robotPosData = getLineFromString(fileContents, 0);
	auto robotPos = getElementsInLineF(robotPosData);
	texts[0].setString("Robot Position: " + std::to_string(robotPos[0]) + ", " + std::to_string(robotPos[1]) + ", " + std::to_string(robotPos[2]));

	std::string robotRotationData = getLineFromString(fileContents, 1);
	auto robotRot = getElementsInLineF(robotRotationData);
	texts[1].setString("Robot Rotation: " + std::to_string(robotRot[0]) + ", " + std::to_string(robotRot[1]) + ", " + std::to_string(robotRot[2]));

	std::string robotWheelData = getLineFromString(fileContents, 2);
	auto robotWheel = getElementsInLineF(robotWheelData);
	texts[2].setString("Left Wheel: " + std::to_string(robotWheel[0]) + ", Right Wheel: " + std::to_string(robotWheel[1]));

	std::string state = getLineFromString(fileContents, 3);
	texts[3].setString("State: " + state);

	std::string robotLeftObstacle = getLineFromString(fileContents, 4);
	auto leftObstacle = getElementsInLineF(robotLeftObstacle);
	texts[4].setString("Left Obstacle Distance: " + std::to_string(leftObstacle[0]));

	std::string robotRightObstacle = getLineFromString(fileContents, 5);
	auto rightObstacle = getElementsInLineF(robotRightObstacle);
	texts[5].setString("Right Obstacle Distance: " + std::to_string(rightObstacle[0]));

	std::string robotRMSEData = getLineFromString(fileContents, 6);
	auto RMSE = getElementsInLineF(robotRMSEData);
	texts[6].setString("RMSE: " + std::to_string(RMSE[0]));

}

int main()
{
	// First, let's find out how big our occupancy grid is, the robot simulator initially writes an empty grid of the correct side to file
	// The grid is a perfect square and 1 pixel is one element in the grid so the image size will be the number of rows * number of columns
	sf::Image mapImage;
	createImage(mapImage);

	sf::RenderWindow window(sf::VideoMode(600, 750), "Robot Map Viewer");

	sf::Texture mapTexture;
	mapTexture.loadFromImage(mapImage);
	sf::Sprite shape(mapTexture);

	shape.setPosition({ 20.f, 240.f });

	sf::Font font;
	font.loadFromFile("arial/arial.ttf");

	std::vector<sf::Text> texts;
	texts.resize(7);
	sf::Vector2f textPostion = { 20.f, 10.f };

	for (auto& text : texts)
	{
		text.setFont(font);
		text.setCharacterSize(24); // in pixels, not points!
		text.setFillColor(sf::Color::White);
		text.setPosition(textPostion);
		textPostion.y += 30.f;
		text.setString("Hello");
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		updateRobotData("../../robotData.txt", texts);
		updateGrid("../../surroundings.txt");
		editImage(mapImage);
		mapTexture.update(mapImage);

		window.clear(sf::Color::Black);
		window.draw(shape);
		for (auto& text : texts)
		{
			window.draw(text);
		}
		window.display();
	}

	return 0;
}