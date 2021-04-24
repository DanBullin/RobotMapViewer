#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

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

int main()
{
	sf::RenderWindow window(sf::VideoMode(1024, 768), "Robot Map Viewer");

	sf::Image mapImage;
	mapImage.create(250, 250, sf::Color::White);
	sf::Texture mapTexture;
	mapTexture.loadFromImage(mapImage);
	sf::Sprite shape(mapTexture);

	shape.setPosition({ 300.f, 300.f });


	std::string fileContents = getContents("../../robot.txt");

	LOG(fileContents);

	while (window.isOpen())
	{
		mapImage.setPixel(100, 100, sf::Color::Red);
		mapTexture.update(mapImage);

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
	}

	return 0;
}