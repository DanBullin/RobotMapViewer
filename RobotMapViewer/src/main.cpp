#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#define LOG(x) std::cout << x << std::endl
sf::Color grey = sf::Color(128, 128, 128);

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

void createImage(sf::Image& image, std::string filePath)
{
	std::string fileContents = getContents(filePath);
	fileContents.erase(std::remove(fileContents.begin(), fileContents.end(), '\n'), fileContents.end()); // Remove end lines

	// CoppeliaSim overwrites the file and there is a brief time when this program will open an empty file, in those particular events, just exit out this function
	if (fileContents.empty() || fileContents == "") return;

	// Size of the texture is the square root of the total number of characters in the string as the map will always be a perfect square
	float size = sqrt(fileContents.size());
	image.create(size, size, grey);

	// Go through the empty grid and initialise with our uncertain colour
	for (int i = 0; i < image.getSize().y; i++)
	{
		for (int j = 0; j < image.getSize().x; j++)
		{
			image.setPixel(j, i, sf::Color::Red);
		}
	}
}

void editImage(sf::Image& image, std::string filePath)
{
	std::string fileContents = getContents(filePath);

	// CoppeliaSim overwrites the file and there is a brief time when this program will open an empty file, in those particular events, just exit out this function
	if (fileContents.empty() || fileContents == "") return;

	// The robot during runtime only writes a small portion of the map to the file for effiency so we need to iterate over that small portion and map it over onto the entire grid
	// The first line of the file contains the top left coordinate in the grid and using the number of elements thereafter to determine just how many of our image we need to edit
	std::string robotData = getLineFromString(fileContents, 0);

	int index = robotData.find(',');
	std::string topLeftXStr = robotData.substr(0, index);
	std::string topLeftYStr = robotData.substr(index+1, robotData.size()-index+1);
	int topLeftX, topLeftY;
	std::istringstream(topLeftXStr) >> topLeftX;
	std::istringstream(topLeftYStr) >> topLeftY;

	fileContents.erase(std::remove(fileContents.begin(), fileContents.end(), '\n'), fileContents.end()); // Remove end lines
	int startingIndex = robotData.size();

	float size = sqrt(fileContents.size() - startingIndex);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			int singleIndex = j + (i * size) + startingIndex;

			if (fileContents.at(singleIndex) == '0')
				image.setPixel(topLeftX +j, topLeftY +i, sf::Color::White);
			else if(fileContents.at(singleIndex) == '1')
				image.setPixel(topLeftX + j, topLeftY + i, sf::Color::Black);
			else
				image.setPixel(topLeftX + j, topLeftY + i, grey);
		}
	}
}

int main()
{
	// First, let's find out how big our occupancy grid is, the robot simulator initially writes an empty grid of the correct side to file
	// The grid is a perfect square and 1 pixel is one element in the grid so the image size will be the number of rows * number of columns
	sf::Image mapImage;
	createImage(mapImage, "../../wholeMap.txt");

	sf::RenderWindow window(sf::VideoMode(1024, 768), "Robot Map Viewer");

	sf::Texture mapTexture;
	mapTexture.loadFromImage(mapImage);
	sf::Sprite shape(mapTexture);

	shape.setPosition({ 20.f, 20.f });
	shape.setScale(1.f, 1.f);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		editImage(mapImage, "../../robotSurroundings.txt");
		mapTexture.update(mapImage);

		window.clear(sf::Color(128, 128, 128));
		window.draw(shape);
		window.display();
	}

	return 0;
}