#include "Algorithms\collision.h"
#include "Algorithms\physics.h"
#include "Camera\camera.h"
#include "Graphics\window.h"
#include "Model Loading\mesh.h"
#include "Model Loading\meshLoaderObj.h"
#include "Model Loading\texture.h"
#include "Objects\alien.h"
#include "Objects\platform.h"
#include "Objects\skybox.h" 
#include "Shaders\shader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// For playing sounds on Windows
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Helper function to play sounds asynchronously
void playSound(const char* filename) {
	PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC);
}

//OBJECT DECLARATIONS AND SETTING UP THE SCENE IS DONE IN MAIN CfPP FOR SIMPLICITY

void processKeyboardInput();
void resetPlayer();

// 0 = None, 1 = Plant, 2 = Fuel (Coin), 3 = Treat
int heldItemID = 0;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool isNearItem = false;

// Task completion tracking
bool taskPlantDelivered = false;
bool taskDogFed = false;
bool taskFuelDelivered = false;
bool taskAlienKilled = false;
int aliensKilledCount = 0;

// Cutscene state
bool cutsceneActive = false;
float cutsceneTimer = 0.0f;
const float CUTSCENE_DURATION = 10.0f; // 10 seconds cutscene
glm::vec3 rocketLaunchOffset = glm::vec3(0.0f); // How much the rocket has moved up
bool rocketLaunched = false;

Window window("Game Engine", 800, 800);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 300.0f, -300.0f);

// Item Positions
glm::vec3 plantPos = glm::vec3(-93.14f, 62.0f, -193.34f);
glm::vec3 fuelPos = glm::vec3(6.33f, 3.5f, -406.48f); 
glm::vec3 treatPos = glm::vec3(105.08f, 50.74f, -199.93f);
// Spaceship Position
glm::vec3 spaceshipPos = glm::vec3(-30.0f, 10.0f, -30.0f);

// Dropped treats from aliens
struct DroppedTreat {
	glm::vec3 pos;
	bool collected;
};
std::vector<DroppedTreat> droppedTreats;

// Centralized collision manager
CollisionManager collisionManager;

// Player physics
PlayerPhysics playerPhysics;

// Spawn position
const glm::vec3 SPAWN_POSITION = glm::vec3(0.0f, 5.0f, 0.0f);

// Game objects, they will be dynamically allocated
Platform* g_platform = nullptr;
Platform* platform1 = nullptr;
Platform* platform2 = nullptr;
Platform* platform3 = nullptr;
Platform* platform4 = nullptr;
Platform* platform5 = nullptr;
Platform* platform6 = nullptr;
Platform* plantPlatform = nullptr; // Platform for the elevated plant
Platform* fence = nullptr;
Platform* mountain1 = nullptr;
Platform* mountain2 = nullptr;
Platform* mountain3 = nullptr;
Platform* mountain4 = nullptr;
Platform* mountain5 = nullptr;
Platform* mountain6 = nullptr;
Platform* spike1 = nullptr;
Platform* spike2 = nullptr;
Platform* spike3 = nullptr;
Platform* spaceshipPlatform = nullptr;
Platform* dogPlatform = nullptr; // Dog Object

std::vector<Alien*> aliens; // Alien enemies

// eye height above player's origin
// player's position is at their feet for collision purposes
// player is just a line segment for collision detection
const float PLAYER_EYE_HEIGHT = 2.0f;

// Helper function to check if all main tasks are complete
bool allMainTasksComplete() {
	return taskPlantDelivered && taskDogFed && taskFuelDelivered && taskAlienKilled;
}

int main()
{
	glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

	//building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
	
	Shader hudShader("Shaders/hud_vertex.glsl", "Shaders/hud_fragment.glsl");

	//Textures
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/fence.bmp"); // fence texture
	GLuint tex3 = loadBMP("Resources/Textures/mars.bmp"); // ground tex
	GLuint tex4 = loadBMP("Resources/Textures/platform.bmp"); // platform tex
	GLuint tex5 = loadBMP("Resources/Textures/rockwall.bmp"); // mountain tex
	GLuint tex6 = loadBMP("Resources/Textures/spikes.bmp");  // spike tex

	GLuint texPlant = loadBMP("Resources/Textures/Plant_BaseColor.bmp");
	GLuint texFuel = loadBMP("Resources/Textures/GoldColor.bmp"); // Fuel texture (was coin)
	GLuint texTreat = loadBMP("Resources/Textures/dogtreat.bmp");
	GLuint texPressE = loadBMP("Resources/Textures/epress.bmp");
	GLuint texSpaceship = loadBMP("Resources/Textures/spaceship_texture.bmp");
	GLuint texAlien = loadBMP("Resources/Textures/alien.bmp");
	GLuint texDog = loadBMP("Resources/Textures/dog.bmp");

	glEnable(GL_DEPTH_TEST);

	// Lock cursor for FPS-style controls
	window.lockCursor(true);

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Texture Vectors
	std::vector<Texture> textures;
	textures.push_back({ tex, "texture_diffuse" });

	// the "fence", big flat box in the middle of the scene
	std::vector<Texture> textures2;
	textures2.push_back({ tex2, "texture_diffuse" });

	// ground texture
	std::vector<Texture> textures3;
	textures3.push_back({ tex3, "texture_diffuse" });

	// platform texture
	std::vector<Texture> textures4;
	textures4.push_back({ tex4, "texture_diffuse" });

	//mountain textures
	std::vector<Texture> textures5;
	textures5.push_back({ tex5, "texture_diffuse" });

	//spike textures
	std::vector<Texture> texturesSpike;
	texturesSpike.push_back({ tex6, "texture_diffuse" });

	std::vector<Texture> texturesPlant;
	texturesPlant.push_back({ texPlant, "texture_diffuse" });

	std::vector<Texture> texturesFuel;
	texturesFuel.push_back({ texFuel, "texture_diffuse" });

	std::vector<Texture> texturesTreat;
	texturesTreat.push_back({ texTreat, "texture_diffuse" });

	std::vector<Texture> texturesPressE;
	texturesPressE.push_back({ texPressE, "texture_diffuse" });

	std::vector<Texture> texturesSpaceship;
	texturesSpaceship.push_back({ texSpaceship, "texture_diffuse" });

	std::vector<Texture> texturesAlien;
	texturesAlien.push_back({ texAlien, "texture_diffuse" });

	std::vector<Texture> texturesDog;
	texturesDog.push_back({ texDog, "texture_diffuse" });

	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);

	// plane mesh for ground
	Mesh plane = loader.loadObj("Resources/Models/plane_mars.obj", textures3);

	// fence mesh
	Mesh fenceMesh = loader.loadObj("Resources/Models/fence.obj", textures2);

	// small platform mesh (reuse plane geometry but with platform texture)
	Mesh platformMesh = loader.loadObj("Resources/Models/floatingplatform.obj", textures4);

	// mountain mesh
	Mesh mountainMesh = loader.loadObj("Resources/Models/Rockwall.obj", textures5);

	// spike mesh
	Mesh spikeMesh = loader.loadObj("Resources/Models/spikes.obj", texturesSpike);

	Mesh plantModel = loader.loadObj("Resources/Models/Plant.obj", texturesPlant);
	Mesh fuelModel = loader.loadObj("Resources/Models/MarioCoin.obj", texturesFuel); // Fuel canister (using coin model)
	Mesh dogTreat = loader.loadObj("Resources/Models/DogTreat.obj", texturesTreat);
	Mesh spaceshipModel = loader.loadObj("Resources/Models/spaceship.obj", texturesSpaceship);
	Mesh alienModel = loader.loadObj("Resources/Models/alien.obj", texturesAlien);
	Mesh dogModel = loader.loadObj("Resources/Models/dog.obj", texturesDog);

	// Platforms Init
	// create a Platform from the plane mesh (keeps rendering + collision logic encapsulated)
	g_platform = new Platform(plane, "Ground");
	g_platform->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
	g_platform->setScale(glm::vec3(300.0f, 2.0f, 1000.0f));

	// create floating platforms in the air
	platform1 = new Platform(platformMesh, "P1");
	platform1->setPosition(glm::vec3(50.0f, 10.0f, -38.0f));
	platform1->setScale(glm::vec3(10.0f, 10.0f, 10.0f));

	platform2 = new Platform(platformMesh, "P2");
	platform2->setPosition(glm::vec3(0.0f, 23.0f, -60.0f));
	platform2->setScale(glm::vec3(15.0f, 15.0f, 15.0f));

	platform3 = new Platform(platformMesh, "P3");
	platform3->setPosition(glm::vec3(40.0f, 40.0f, -110.0f));
	platform3->setScale(glm::vec3(20.0f, 10.0f, 25.0f));

	platform4 = new Platform(platformMesh, "P4");
	platform4->setPosition(glm::vec3(9.0f, 50.0f, -150.0f));
	platform4->setScale(glm::vec3(10.0f, 10.0f, 10.0f));

	platform5 = new Platform(platformMesh, "P5");
	platform5->setPosition(glm::vec3(110.0f, 30.0f, -260.0f));
	platform5->setScale(glm::vec3(13.0f, 10.0f, 13.0f));

	platform6 = new Platform(platformMesh, "P6");
	platform6->setPosition(glm::vec3(76.0f, 20.0f, -242.0f));
	platform6->setScale(glm::vec3(13.0f, 10.0f, 13.0f));

	// Plant Platform initialization
	plantPlatform = new Platform(platformMesh, "PlantPlatform");
	plantPlatform->setPosition(glm::vec3(-93.14f, 60.0f, -193.34f)); 
	plantPlatform->setScale(glm::vec3(10.0f, 5.0f, 10.0f));

	//fence in the middle of the scene, big flat box
	fence = new Platform(fenceMesh, "fence");
	fence->setPosition(glm::vec3(60.0f, 0.0f, -200.0f));
	fence->setScale(glm::vec3(175.0f, 25.0f, 20.0f));

	//mountain around gameplay area acting as walls
	mountain1 = new Platform(mountainMesh, "M1");
	mountain1->setPosition(glm::vec3(100.0f, 0.0f, 100.0f));
	mountain1->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
	mountain1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
	mountain1->setUseOBBCollision(true);

	mountain2 = new Platform(mountainMesh, "M2");
	mountain2->setPosition(glm::vec3(-100.0f, 0.0f, 100.0f));
	mountain2->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
	mountain2->setRotation(glm::vec3(0.0f, -180.0f, 0.0f));
	mountain2->setUseOBBCollision(true);

	mountain4 = new Platform(mountainMesh, "M4");
	mountain4->setPosition(glm::vec3(223.0f, 0.0f, -275.0f));
	mountain4->setScale(glm::vec3(300.0f, 270.0f, 300.0f));
	mountain4->setRotation(glm::vec3(0.0f, 80.0f, 0.0f));
	mountain4->setUseOBBCollision(true);

	mountain5 = new Platform(mountainMesh, "M5");
	mountain5->setPosition(glm::vec3(-223.0f, 0.0f, -275.0f));
	mountain5->setScale(glm::vec3(300.0f, 350.0f, 300.0f));
	mountain5->setRotation(glm::vec3(0.0f, 80.0f, 0.0f));
	mountain5->setUseOBBCollision(true);

	mountain3 = new Platform(mountainMesh, "M3");
	mountain3->setPosition(glm::vec3(0.0f, 0.0f, 150.0f));
	mountain3->setScale(glm::vec3(130.0f, 200.0f, 130.0f));

	mountain6 = new Platform(mountainMesh, "M6");
	mountain6->setPosition(glm::vec3(50.0f, 0.0f, -470.0f));
	mountain6->setScale(glm::vec3(300.0f, 450.0f, 200.0f));

	spike1 = new Platform(spikeMesh, "S1");
	spike1->setPosition(glm::vec3(-100.0f, 1.0f, -200.0f));
	spike1->setScale(glm::vec3(50.0f, 5.0f, 25.0f));
	//if hazard, player will reset upon collision
	spike1->setIsHazard(true);

	spike2 = new Platform(spikeMesh, "S2");
	spike2->setPosition(glm::vec3(37.0f, 42.0f, -115.0f));
	spike2->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
	spike2->setIsHazard(true);

	spike3 = new Platform(spikeMesh, "S3");
	spike3->setPosition(glm::vec3(9.0f, 49.0f, -196.0f));
	spike3->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
	spike3->setIsHazard(true);

	// Spaceship as a Platform for Collision
	spaceshipPlatform = new Platform(spaceshipModel, "Spaceship");
	spaceshipPlatform->setPosition(spaceshipPos);
	spaceshipPlatform->setScale(glm::vec3(10.0f, 10.0f, 10.0f));
	spaceshipPlatform->setUseOBBCollision(true);

	// Dog Object 
	dogPlatform = new Platform(dogModel, "Dog");
	glm::vec3 dogPos = spaceshipPos;
	dogPos.x += 15.0f; // Next to ship
	dogPos.y = 3.0f;   // Lower to ground (Fix floating)
	dogPlatform->setPosition(dogPos);
	dogPlatform->setScale(glm::vec3(3.0f, 3.0f, 3.0f)); // Adjust scale as needed
	dogPlatform->setUseOBBCollision(true);

	// Register all platforms with the collision manager
	collisionManager.addCollidable(g_platform);
	collisionManager.addCollidable(platform1);
	collisionManager.addCollidable(platform2);
	collisionManager.addCollidable(platform3);
	collisionManager.addCollidable(platform4);
	collisionManager.addCollidable(platform5);
	collisionManager.addCollidable(platform6);
	collisionManager.addCollidable(plantPlatform); 
	collisionManager.addCollidable(fence);
	collisionManager.addCollidable(mountain1);
	collisionManager.addCollidable(mountain2);
	collisionManager.addCollidable(mountain3);
	collisionManager.addCollidable(mountain4);
	collisionManager.addCollidable(mountain5);
	collisionManager.addCollidable(mountain6);
	collisionManager.addCollidable(spike1);
	collisionManager.addCollidable(spike2);
	collisionManager.addCollidable(spike3);
	collisionManager.addCollidable(spaceshipPlatform);
	collisionManager.addCollidable(dogPlatform);

	// 0 makes most sense for our game
	collisionManager.setCollisionMargin(0);
	// Disable verbose debug output in release runs to avoid console flooding and input lag
	collisionManager.setDebugOutput(false);  // Changed from true to false to reduce mouse-look lag



	aliens.push_back(new Alien(&alienModel, glm::vec3(100.0f, 55.0f, -195.0f), 15.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(110.0f, 55.0f, -195.0f), 15.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(100.0f, 55.0f, -205.0f), 15.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(110.0f, 55.0f, -205.0f), 15.0f));

	aliens.push_back(new Alien(&alienModel, glm::vec3(-0.03f, 32.0f, -63.59f), 20.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(-27.32f, 1.0f, -379.37f), 25.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(1.70f, 1.0f, -340.87f), 25.0f));
	aliens.push_back(new Alien(&alienModel, glm::vec3(66.89f, 1.0f, -388.61f), 25.0f));

	aliens.push_back(new Alien(&alienModel, glm::vec3(-53.96, 1.0, -279.16), 25.0f));
	std::vector<Vertex> hudVerts;
	std::vector<int> hudIndices = { 0, 1, 2, 0, 2, 3 };
	Vertex v;
	v.textureCoords = glm::vec2(0, 0); v.pos = glm::vec3(-0.02f, 0.02f, 0.0f); hudVerts.push_back(v);
	v.textureCoords = glm::vec2(1, 0); v.pos = glm::vec3(0.02f, 0.02f, 0.0f); hudVerts.push_back(v);
	v.textureCoords = glm::vec2(1, 1); v.pos = glm::vec3(0.02f, -0.02f, 0.0f); hudVerts.push_back(v);
	v.textureCoords = glm::vec2(0, 1); v.pos = glm::vec3(-0.02f, -0.02f, 0.0f); hudVerts.push_back(v);
	Mesh hudSquare(hudVerts, hudIndices, texturesPressE);

	// Create skybox:
	Skybox skybox;
	std::vector<std::string> skyboxFaces = {
		"Resources/Textures/red/bkg1_right1.bmp",
		"Resources/Textures/red/bkg1_left2.bmp",
		"Resources/Textures/red/bkg1_top3.bmp",
		"Resources/Textures/red/bkg1_bottom4.bmp",
		"Resources/Textures/red/bkg1_front5.bmp",
		"Resources/Textures/red/bkg1_back6.bmp"
	};
	skybox.load(skyboxFaces);


	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		playerPhysics.printStatus(camera.getCameraPosition());
		// Handle cutscene
		if (cutsceneActive)
		{
			// Only remove and delete dogPlatform once
			if (dogPlatform)
			{
				collisionManager.removeCollidable(dogPlatform);
				delete dogPlatform;
				dogPlatform = nullptr;
			}
			cutsceneTimer += deltaTime;

			// Unlock cursor during cutscene so it doesn't interfere
			window.lockCursor(false);

			// Position camera to the side of the rocket for a cinematic view
			glm::vec3 rocketCurrentPos = spaceshipPos + rocketLaunchOffset;
			glm::vec3 cutsceneCamPos = glm::vec3(120.0f, 20.0f, 0.0f); // Side view
			glm::vec3 cutsceneLookAt = glm::vec3(0.0f, 20.0f, 0.0f); // Look at rocket center

			camera.setCameraPosition(cutsceneCamPos);
			// Calculate direction to look at rocket
			glm::vec3 lookDir = glm::normalize(cutsceneLookAt - cutsceneCamPos);
			camera.setCameraViewDirection(lookDir);

			// After a short delay, start the rocket launch
			if (cutsceneTimer > 1.0f && !rocketLaunched)
			{
				rocketLaunched = true;
				printf("ROCKET LAUNCHING!\n");
			}

			// Accelerate the rocket upward

			if (rocketLaunched)
			{
				float launchSpeed = 50.0f + (cutsceneTimer - 1.0f) * 80.0f; // Accelerating
				rocketLaunchOffset.y += launchSpeed * deltaTime;

				// Update spaceship platform position
				if (spaceshipPlatform)
				{
					spaceshipPlatform->setPosition(spaceshipPos + rocketLaunchOffset);
				}
			}

			// End cutscene after duration
			if (cutsceneTimer >= CUTSCENE_DURATION)
			{
				cutsceneActive = false;
				window.lockCursor(true);
				printf("Cutscene ended! You escaped the planet!\n");
			}
		}
		else
		{
			processKeyboardInput();
		}

		
		
		
		//// Code for the light ////

		sunShader.use();
		//camera fov
		glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(70.0f), window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());
		skybox.draw(ViewMatrix, ProjectionMatrix);

		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		//Test for one Obj loading = light source
		glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0), lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		sun.draw(sunShader);

		//// End code for the light ////

		shader.use();
		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// Box
		ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		box.draw(shader);

		///// Draw platforms via class //////
		// main ground platform
		g_platform->draw(shader, ViewMatrix, ProjectionMatrix);
		// floating platforms
		if (platform1) platform1->draw(shader, ViewMatrix, ProjectionMatrix);
		if (platform2) platform2->draw(shader, ViewMatrix, ProjectionMatrix);
		if (platform3) platform3->draw(shader, ViewMatrix, ProjectionMatrix);
		if (platform4) platform4->draw(shader, ViewMatrix, ProjectionMatrix);
		if (platform5) platform5->draw(shader, ViewMatrix, ProjectionMatrix);
		if (platform6) platform6->draw(shader, ViewMatrix, ProjectionMatrix);
		if (plantPlatform) plantPlatform->draw(shader, ViewMatrix, ProjectionMatrix); // Draw call
		if (fence) fence->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain1) mountain1->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain2) mountain2->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain3) mountain3->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain4) mountain4->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain5) mountain5->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain6) mountain6->draw(shader, ViewMatrix, ProjectionMatrix);

		// In the render loop, after drawing other platforms:
		if (spike1) spike1->draw(shader, ViewMatrix, ProjectionMatrix);
		if (spike2) spike2->draw(shader, ViewMatrix, ProjectionMatrix);
		if (spike3) spike3->draw(shader, ViewMatrix, ProjectionMatrix);

		if (spaceshipPlatform) spaceshipPlatform->draw(shader, ViewMatrix, ProjectionMatrix);

		// Draw dog if it exists
		if (dogPlatform) dogPlatform->draw(shader, ViewMatrix, ProjectionMatrix);

		// Aliens
		for (auto& alien : aliens) {
			alien->update(deltaTime, collisionManager);
			alien->draw(shader, ViewMatrix, ProjectionMatrix);
			bool stomped = false;
			if (alien->checkPlayerCollision(camera.getCameraPosition(), playerPhysics.velocity, stomped)) {
				// Player died
				resetPlayer();
			}
			else if (stomped) {
				playSound("Resources/sleep_short.wav");
				// Track alien kills for task
				aliensKilledCount++;
				if (!taskAlienKilled) {
					taskAlienKilled = true;
					printf("Task Complete: Kill an Alien!\n");
				}

				// Create dropped treat
				DroppedTreat t;
				t.pos = alien->getPosition();
				t.pos.y += 2.0f;
				t.collected = false;
				droppedTreats.push_back(t);

				// Bounce player
				playerPhysics.velocity.y = 10.0f; // Bounce
			}
		}

		// --- Items ---
		// Only draw plant if not delivered
		if (!taskPlantDelivered) {
			if (heldItemID == 1) {
				ModelMatrix = glm::translate(glm::mat4(1.0), camera.getCameraPosition() + (camera.getCameraViewDirection() * 1.5f) - (camera.getCameraUp() * 0.5f) + (glm::cross(camera.getCameraViewDirection(), camera.getCameraUp()) * 0.8f));
			}
			else {
				float plantWobble = sin(currentFrame * 2.0f) * 0.5f;
				ModelMatrix = glm::translate(glm::mat4(1.0), plantPos + glm::vec3(0, plantWobble, 0));
			}
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			plantModel.draw(shader);
		}

		// Only draw fuel if not delivered
		if (!taskFuelDelivered) {
			if (heldItemID == 2) {
				ModelMatrix = glm::translate(glm::mat4(1.0), camera.getCameraPosition() + (camera.getCameraViewDirection() * 1.5f) - (camera.getCameraUp() * 0.5f) + (glm::cross(camera.getCameraViewDirection(), camera.getCameraUp()) * 0.8f));
			}
			else {
				float fuelWobble = sin(currentFrame * 3.0f) * 0.5f;
				ModelMatrix = glm::translate(glm::mat4(1.0), fuelPos + glm::vec3(0, fuelWobble, 0));
			}
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 150.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			fuelModel.draw(shader);
		}

		// Only draw treat if not fed to dog (and not holding it)
		if (!taskDogFed) {
			if (heldItemID == 3) {
				ModelMatrix = glm::translate(glm::mat4(1.0), camera.getCameraPosition() + (camera.getCameraViewDirection() * 1.5f) - (camera.getCameraUp() * 0.5f) + (glm::cross(camera.getCameraViewDirection(), camera.getCameraUp()) * 0.8f));
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				dogTreat.draw(shader);
			}
			else if (treatPos.y > 0) {
				float treatWobble = sin(currentFrame * 1.5f) * 0.5f;
				ModelMatrix = glm::translate(glm::mat4(1.0), treatPos + glm::vec3(0, treatWobble, 0));
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				dogTreat.draw(shader);
			}
		}

		// Draw Dropped Treats (only if dog not fed yet)
		if (!taskDogFed) {
			for (auto& t : droppedTreats) {
				if (t.collected)
					continue;

				float treatWobble = sin(currentFrame * 1.5f) * 0.5f;
				ModelMatrix = glm::translate(glm::mat4(1.0), t.pos + glm::vec3(0, treatWobble, 0));
				ModelMatrix = glm::rotate(ModelMatrix, glm::radians(currentFrame * 40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
				dogTreat.draw(shader);
			}
		}

		// HUD
		bool nearAnyTreat = false;
		if (!droppedTreats.empty() && !taskDogFed) {
			for (auto& t : droppedTreats) {
				if (!t.collected && glm::distance(camera.getCameraPosition(), t.pos) < 5.0f) {
					nearAnyTreat = true;
					break;
				}
			}
		}

		// Distance calculations for HUD
		float distToShip = glm::distance(camera.getCameraPosition(), spaceshipPos);
		float distToDog = 1000.0f;
		if (dogPlatform)
			distToDog = glm::distance(camera.getCameraPosition(), dogPlatform->getPosition());

		float shipDepositDist = 12.0f;
		float dogDepositDist = 12.0f;

		// Show "Press E" when near ship and all tasks complete (final task)
		bool nearShipForFinalTask = allMainTasksComplete() && distToShip < shipDepositDist;

		// Show "Press E" when holding plant or fuel and near ship
		bool canDepositAtShip = (heldItemID == 1 || heldItemID == 2) && distToShip < shipDepositDist;

		// Show "Press E" when holding treat and near dog
		bool canFeedDog = (heldItemID == 3) && distToDog < dogDepositDist;

		// Show "Press E" when near items to pick up (and not holding anything)
		bool canPickUpItem = (isNearItem || nearAnyTreat) && heldItemID == 0;

		// Combined condition for showing "Press E"
		bool showPressE = canPickUpItem || canDepositAtShip || canFeedDog || nearShipForFinalTask;

		if (showPressE) {
			glDisable(GL_DEPTH_TEST);
			hudShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texPressE);
			glUniform1i(glGetUniformLocation(hudShader.getId(), "texture1"), 0);
			glm::mat4 hudProj = glm::mat4(1.0);
			glm::mat4 hudView = glm::mat4(1.0);
			glm::mat4 hudModel = glm::mat4(1.0);
			hudModel = glm::translate(hudModel, glm::vec3(0.0f, -0.8f, 0.0f));
			hudModel = glm::scale(hudModel, glm::vec3(15.0f, 4.0f, 1.0f));
			GLuint hudMatrixID = glGetUniformLocation(hudShader.getId(), "MVP");
			glm::mat4 hudMVP = hudProj * hudView * hudModel;
			glUniformMatrix4fv(hudMatrixID, 1, GL_FALSE, &hudMVP[0][0]);
			hudSquare.draw(hudShader);
			glEnable(GL_DEPTH_TEST);
		}

		// ImGui Task List HUD
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Position window in top-left corner
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.7f); // Semi-transparent background

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;

		ImGui::Begin("Tasks", nullptr, window_flags);

		ImGui::Text("=== MISSION OBJECTIVES ===");
		ImGui::Separator();

		// Task 1: Deliver plant to ship
		if (taskPlantDelivered)
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[X] Deliver Plant to Ship");
		else if (heldItemID == 1)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[~] Bring Plant to Ship");
		else
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ ] Collect Plant");

		// Task 2: Deliver fuel to ship
		if (taskFuelDelivered)
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[X] Deliver Fuel to Ship");
		else if (heldItemID == 2)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[~] Bring Fuel to Ship");
		else
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ ] Collect Fuel");

		// Task 3: Feed the dog
		if (taskDogFed)
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[X] Feed the Dog");
		else if (heldItemID == 3)
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[~] Bring Treat to Dog");
		else
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ ] Collect Treat");

		// Task 4: Kill an alien
		if (taskAlienKilled)
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[X] Defeat an Alien");
		else
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ ] Defeat an Alien (Stomp!)");

		ImGui::Separator();

		// Final Task: Only show when all other tasks are complete
		if (allMainTasksComplete()) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), ">>> BOARD THE SHIP TO ESCAPE! <<<");
		}
		else {
			ImGui::Text("Complete all tasks to escape!");
		}

		ImGui::Separator();
		ImGui::Text("Press E to interact");
		ImGui::Text("Stomp aliens to defeat them!");

		ImGui::End();
		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		window.update();
	}

	// Remove from collision manager before deleting
	collisionManager.clearAll();
	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Remove from collision manager before deleting
	collisionManager.clearAll();

	// cleanup
	delete g_platform;
	g_platform = nullptr;
	delete platform1;
	platform1 = nullptr;
	delete platform2;
	platform2 = nullptr;
	delete platform3;
	platform3 = nullptr;
	delete platform4;
	platform4 = nullptr;
	delete platform5;
	platform5 = nullptr;
	delete platform6;
	platform6 = nullptr;
	delete plantPlatform; // Cleanup
	plantPlatform = nullptr;
	delete fence;
	fence = nullptr;
	delete mountain1;
	mountain1 = nullptr;
	delete mountain2;
	mountain2 = nullptr;
	delete mountain3;
	mountain3 = nullptr;
	delete mountain4;
	mountain4 = nullptr;
	delete mountain5;
	mountain5 = nullptr;
	delete mountain6;
	mountain6 = nullptr;
	delete spike1;
	spike1 = nullptr;
	delete spike2;
	spike2 = nullptr;
	delete spike3;
	spike3 = nullptr;
	delete spaceshipPlatform;
	spaceshipPlatform = nullptr;
	delete dogPlatform;
	dogPlatform = nullptr;

	for (auto a : aliens)
		delete a;
}

//basically game loop input processing
void processKeyboardInput()
{
	float distPlant = glm::distance(camera.getCameraPosition(), plantPos);
	float distFuel = glm::distance(camera.getCameraPosition(), fuelPos);
	float distTreat = glm::distance(camera.getCameraPosition(), treatPos);
	// Distance to spaceship
	float distToShip = glm::distance(camera.getCameraPosition(), spaceshipPos);
	// Dog Interaction
	float distToDog = 1000.0f;
	if (dogPlatform)
		distToDog = glm::distance(camera.getCameraPosition(), dogPlatform->getPosition());

	float interactionDist = 5.0f;
	float shipDepositDist = 12.0f; // Slightly larger because the ship is big

	bool nearStaticItem = false;
	if (!taskPlantDelivered && distPlant < interactionDist) nearStaticItem = true;
	if (!taskFuelDelivered && distFuel < interactionDist) nearStaticItem = true;
	if (!taskDogFed && distTreat < interactionDist) nearStaticItem = true;
	isNearItem = nearStaticItem;

	static bool eWasPressed = false;
	if (window.isPressed(GLFW_KEY_E) && !eWasPressed) {
		if (heldItemID == 0) {
			// Pick up items (only if task not complete)
			if (!taskPlantDelivered && distPlant < interactionDist)
				heldItemID = 1;
			else if (!taskFuelDelivered && distFuel < interactionDist)
				heldItemID = 2;
			else if (!taskDogFed && distTreat < interactionDist)
				heldItemID = 3;
			else if (!taskDogFed) {
				// Check dropped treats
				for (auto& t : droppedTreats) {
					if (!t.collected && glm::distance(camera.getCameraPosition(), t.pos) < interactionDist) {
						t.collected = true;
						heldItemID = 3;
						break;
					}
				}
			}

			// Final task: Board the ship when all tasks complete
			if (allMainTasksComplete() && distToShip < shipDepositDist) {
				printf("All tasks complete! Launching rocket!\n");
				cutsceneActive = true;
				cutsceneTimer = 0.0f;
				rocketLaunched = false;
			}
		}
		else {
			// --- DEPOSIT LOGIC ---
			// Deposit plant at ship
			if (heldItemID == 1 && distToShip < shipDepositDist) {
				taskPlantDelivered = true;
				heldItemID = 0;
				printf("Task Complete: Plant delivered to spaceship!\n");
			}
			// Deposit fuel at ship
			else if (heldItemID == 2 && distToShip < shipDepositDist) {
				taskFuelDelivered = true;
				heldItemID = 0;
				printf("Task Complete: Fuel delivered to spaceship!\n");
			}
			// Give treat to Dog
			else if (heldItemID == 3 && distToDog < shipDepositDist) {
				taskDogFed = true;
				heldItemID = 0;
				printf("Task Complete: Treat given to Dog! Good boy!\n");
			}
			else {
				// Normal drop logic for everything else (or if not near ship/dog)
				glm::vec3 dropPos = camera.getCameraPosition() + (camera.getCameraViewDirection() * 3.0f);
				if (camera.getCameraPosition().y < 10.0f)
					dropPos.y = 2.5f;
				else
					dropPos.y = camera.getCameraPosition().y - 0.5f;

				if (heldItemID == 1)
					plantPos = dropPos;
				if (heldItemID == 2)
					fuelPos = dropPos;
				if (heldItemID == 3)
					treatPos = dropPos;
				heldItemID = 0;
			}
		}
	}
	eWasPressed = window.isPressed(GLFW_KEY_E);

	// lock cursor in window with tab key 
	static bool tabWasPressed = false;
	bool tabPressed = window.isPressed(GLFW_KEY_TAB);
	if (tabPressed && !tabWasPressed)
	{
		window.lockCursor(!window.isCursorLocked());
	}
	tabWasPressed = tabPressed;

	// mouse look
	if (window.isCursorLocked())
	{
		double deltaX, deltaY;
		window.getMouseDelta(deltaX, deltaY);
		camera.mouseLook((float)deltaX, (float)deltaY, 0.1f);
		window.resetMouseDelta();
	}

	// debug: reset player position with P key
	static bool pWasPressed = false;
	if (window.isPressed(GLFW_KEY_P) && !pWasPressed)
	{
		resetPlayer();
	}
	pWasPressed = window.isPressed(GLFW_KEY_P);

	// Get current position
	glm::vec3 currentPos = camera.getCameraPosition();

	// calculate movement direction based on input
	glm::vec3 moveDirection(0.0f);

	// get forward and right vectors from camera (ignore y component for movement)
	glm::vec3 forward = camera.getCameraViewDirection();
	forward.y = 0.0f;
	if (glm::length(forward) > 0.0001f)
		forward = glm::normalize(forward);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

	// WASD movement input
	if (window.isPressed(GLFW_KEY_W))
		moveDirection += forward;
	if (window.isPressed(GLFW_KEY_S))
		moveDirection -= forward;
	if (window.isPressed(GLFW_KEY_A))
		moveDirection -= right;
	if (window.isPressed(GLFW_KEY_D))
		moveDirection += right;

	// Check if running (holding shift)
	bool isRunning = window.isPressed(GLFW_KEY_LEFT_SHIFT) || window.isPressed(GLFW_KEY_RIGHT_SHIFT);

	// Apply movement input to physics
	playerPhysics.applyMovementInput(moveDirection, isRunning);

	// Jump (spacebar) - use edge detection AND require grounded
	static bool spaceWasPressed = false;
	if (window.isPressed(GLFW_KEY_SPACE) && !spaceWasPressed && playerPhysics.isGrounded)
	{
		playerPhysics.jump();
	}
	spaceWasPressed = window.isPressed(GLFW_KEY_SPACE);

	// Update physics and get movement delta
	glm::vec3 delta = playerPhysics.update(deltaTime);

	// Apply delta to get proposed position
	glm::vec3 proposed = currentPos + delta;

	// Check collision with all objects
	bool collided = collisionManager.resolvePointAgainstAll(proposed, PLAYER_EYE_HEIGHT);

	bool groundContactThisFrame = false;

	if (collided)
	{
		const auto& info = collisionManager.getLastCollisionInfo();

		// Check for hazard collision (no dynamic_cast needed)
		if (info.collidable && info.collidable->isHazard())
		{
			resetPlayer();
			return;
		}

		// Handle collision based on face
		if (info.face == CollisionManager::ContactFace::Top)
		{
			playerPhysics.onGroundCollision(proposed.y);
			groundContactThisFrame = true;
		}
		else if (info.face == CollisionManager::ContactFace::Bottom)
		{
			// Only call ceiling collision for actual ceiling hits
			playerPhysics.onCeilingCollision();
		}
		else
		{
			// For ANY collision (including side walls), check if we're still on ground
			// This is crucial, wall collisions should NOT break ground contact
			glm::vec3 probePos = proposed;
			probePos.y -= 0.2f;

			if (collisionManager.resolvePointAgainstAll(probePos, PLAYER_EYE_HEIGHT))
			{
				if (collisionManager.getLastCollisionInfo().face == CollisionManager::ContactFace::Top)
				{
					groundContactThisFrame = true;
					playerPhysics.isGrounded = true;
				}
			}
		}
	}
	else
	{
		// No collision, check if we're still on ground (edge detection)
		if (playerPhysics.isGrounded && playerPhysics.velocity.y <= 0.1f)
		{
			glm::vec3 probePos = proposed;
			probePos.y -= 0.3f;

			if (collisionManager.resolvePointAgainstAll(probePos, PLAYER_EYE_HEIGHT))
			{
				if (collisionManager.getLastCollisionInfo().face == CollisionManager::ContactFace::Top)
				{
					groundContactThisFrame = true;
					playerPhysics.isGrounded = true;
				}
			}
		}
	}

	// update grounded state
	if (!groundContactThisFrame)
	{
		playerPhysics.isGrounded = false;
	}


	// apply the resolved position to the camera
	camera.setCameraPosition(proposed);
}

void resetPlayer()
{
	playSound("Resources/sleep_short.wav");
	camera.setCameraPosition(SPAWN_POSITION);
	playerPhysics.reset();
}
