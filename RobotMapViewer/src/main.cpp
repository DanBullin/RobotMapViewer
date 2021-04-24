#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#define LOG(x) std::cout << x << std::endl

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

void editImage(sf::Image& image, bool initialised)
{
	std::string fileContents = getContents("../../robot.txt");
	fileContents.erase(std::remove(fileContents.begin(), fileContents.end(), '\n'), fileContents.end()); // Remove end lines
	sf::Color grey = sf::Color(128, 128, 128);

	if (!initialised)
	{
		uint32_t size = sqrt(fileContents.size());
		image.create(size, size, grey);
	}

	for (int i = 0; i < image.getSize().y; i++)
	{
		for (int j = 0; j < image.getSize().x; j++)
		{
			int singleIndex = j + (i * image.getSize().x);
			if (fileContents.at(singleIndex) == '0')
				image.setPixel(j, i, sf::Color::White);
			else if(fileContents.at(singleIndex) == '1')
				image.setPixel(j, i, sf::Color::Black);
			else
				image.setPixel(j, i, grey);
		}
	}
}

int main()
{
	// First, let's find out how big our occupancy grid is, the robot simulator initially writes an empty grid of the correct side to file
	// The grid is a perfect square and 1 pixel is one element in the grid so the image size will be the number of rows * number of columns
	sf::Image mapImage;
	editImage(mapImage, false);

	sf::RenderWindow window(sf::VideoMode(1024, 768), "Robot Map Viewer");

	sf::Texture mapTexture;
	mapTexture.loadFromImage(mapImage);
	sf::Sprite shape(mapTexture);

	shape.setPosition({ 20.f, 20.f });
	shape.setScale(20.f, 20.f);

	while (window.isOpen())
	{
		editImage(mapImage, true);
		mapTexture.update(mapImage);

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear(sf::Color(128, 128, 128));
		window.draw(shape);
		window.display();
	}

	return 0;
}