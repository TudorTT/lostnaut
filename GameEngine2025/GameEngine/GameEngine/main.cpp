#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include "Objects\platform.h"
#include "Algorithms\collision.h"
#include "Algorithms\physics.h"

void processKeyboardInput();
void resetPlayer();  

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Window window("Game Engine", 800, 800);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 300.0f, -300.0f);

// Centralized collision manager
CollisionManager collisionManager;

// Player physics
PlayerPhysics playerPhysics;

// Spawn position
const glm::vec3 SPAWN_POSITION = glm::vec3(0.0f, 5.0f, 0.0f);

// simple global platform pointer used by the input/physics clamp below
Platform* g_platform = nullptr;
Platform* platform1 = nullptr;
Platform* platform2 = nullptr;
Platform* platform3 = nullptr;
Platform* platform4 = nullptr;
Platform* platform5 = nullptr;
Platform* platform6 = nullptr;

Platform* fence = nullptr;

Platform* mountain1 = nullptr;
Platform* mountain2 = nullptr;
Platform* mountain3 = nullptr;
Platform* mountain4 = nullptr;
Platform* mountain5 = nullptr;
Platform* mountain6 = nullptr;
Platform* mountainfence = nullptr;

// NEW: Spike hazards
Platform* spike1 = nullptr;
Platform* spike2 = nullptr;
Platform* spike3 = nullptr;

// eye height above player's origin
const float PLAYER_EYE_HEIGHT = 1.0f;

int main()
{
	glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

	//building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	//Textures
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/fence.bmp"); // fence texture
	GLuint tex3 = loadBMP("Resources/Textures/mars.bmp"); // ground tex
	GLuint tex4 = loadBMP("Resources/Textures/platform.bmp"); // platform tex
	GLuint tex5 = loadBMP("Resources/Textures/rockwall.bmp"); // moutain tex
	GLuint tex6 = loadBMP("Resources/Textures/spikes.bmp");  // spike tex

	glEnable(GL_DEPTH_TEST);

	// Lock cursor for FPS-style controls
	window.lockCursor(true);

	//Test custom mesh loading
	std::vector<Vertex> vert;
	vert.push_back(Vertex());
	vert[0].pos = glm::vec3(10.5f, 10.5f, 0.0f);
	vert[0].textureCoords = glm::vec2(1.0f, 1.0f);

	vert.push_back(Vertex());
	vert[1].pos = glm::vec3(10.5f, -10.5f, 0.0f);
	vert[1].textureCoords = glm::vec2(1.0f, 0.0f);

	vert.push_back(Vertex());
	vert[2].pos = glm::vec3(-10.5f, -10.5f, 0.0f);
	vert[2].textureCoords = glm::vec2(0.0f, 0.0f);

	vert.push_back(Vertex());
	vert[3].pos = glm::vec3(-10.5f, 10.5f, 0.0f);
	vert[3].textureCoords = glm::vec2(0.0f, 1.0f);

	vert[0].normals = glm::normalize(glm::cross(vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
	vert[1].normals = glm::normalize(glm::cross(vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
	vert[2].normals = glm::normalize(glm::cross(vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
	vert[3].normals = glm::normalize(glm::cross(vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

	std::vector<int> ind = { 0, 1, 3,
		1, 2, 3 };

	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = tex;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	std::vector<Texture> textures4;
	textures4.push_back(Texture());
	textures4[0].id = tex4;
	textures4[0].type = "texture_diffuse";

	std::vector<Texture> textures5;
	textures5.push_back(Texture());
	textures5[0].id = tex5;
	textures5[0].type = "texture_diffuse";

	// Add spike texture vector
	std::vector<Texture> texturesSpike;
	texturesSpike.push_back(Texture());
	texturesSpike[0].id = tex6;
	texturesSpike[0].type = "texture_diffuse";


	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh box = loader.loadObj("Resources/Models/cube.obj", textures);
	Mesh plane = loader.loadObj("Resources/Models/plane_mars.obj", textures3);
	Mesh fenceMesh = loader.loadObj("Resources/Models/fence.obj", textures2);
	// small platform mesh (reuse plane geometry but with platform texture)
	Mesh platformMesh = loader.loadObj("Resources/Models/floatingplatform.obj", textures4);
	Mesh mountainMesh = loader.loadObj("Resources/Models/Rockwall.obj", textures5);
	// create a Platform from the plane mesh (keeps rendering + collision logic encapsulated)
	g_platform = new Platform(plane, "Ground");
	g_platform->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
	// scale the plane larger so it's easier to stand on
	g_platform->setScale(glm::vec3(300.0f, 2.0f, 1000.0f));

	// create floating platforms in the air
	platform1 = new Platform(platformMesh, "FloatingPlatform1");
	platform1->setPosition(glm::vec3(50.0f, 10.0f, -38.0f));
	platform1->setScale(glm::vec3(10.0f, 10.0f, 10.0f));

	platform2 = new Platform(platformMesh, "FloatingPlatform2");
	platform2->setPosition(glm::vec3(0.0f, 30.0f,-60.0f));
	platform2->setScale(glm::vec3(15.0f, 15.0f, 15.0f));

	platform3 = new Platform(platformMesh, "FloatingPlatform3");
	platform3->setPosition(glm::vec3(40.0f, 40.0f, -110.0f));
	platform3->setScale(glm::vec3(20.0f, 10.0f, 25.0f));

	platform4 = new Platform(platformMesh, "FloatingPlatform4");
	platform4->setPosition(glm::vec3(9.0f, 50.0f, -150.0f));
	platform4->setScale(glm::vec3(10.0f, 10.0f, 10.0f));

	platform5 = new Platform(platformMesh, "FloatingPlatform5");
	platform5->setPosition(glm::vec3(110.0f, 30.0f, -260.0f));
	platform5->setScale(glm::vec3(13.0f, 10.0f, 13.0f));

	platform6 = new Platform(platformMesh, "FloatingPlatform6");
	platform6->setPosition(glm::vec3(76.0f, 20.0f, -242.0f));
	platform6->setScale(glm::vec3(13.0f, 10.0f, 13.0f));



	fence = new Platform(fenceMesh, "fence");
	fence->setPosition(glm::vec3(60.0f, -1.0f, -200.0f));
	fence->setScale(glm::vec3(175.0f, 25.0f, 20.0f));



	mountain1 = new Platform(mountainMesh, "Mountain1");
	mountain1->setPosition(glm::vec3(100.0f, 0.0f, 100.0f));
	mountain1->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
	mountain1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
	mountain1->setUseOBBCollision(true);

	mountain2 = new Platform(mountainMesh, "Mountain2");
	mountain2->setPosition(glm::vec3(-100.0f, 0.0f, 100.0f));
	mountain2->setScale(glm::vec3(250.0f, 250.0f, 250.0f));
	mountain2->setRotation(glm::vec3(0.0f, -180.0f, 0.0f));
	mountain2->setUseOBBCollision(true);

	mountain3 = new Platform(mountainMesh, "Mountain3");
	mountain3->setPosition(glm::vec3(0.0f, 0.0f, 150.0f));
	mountain3->setScale(glm::vec3(130.0f, 130.0f, 130.0f));
	mountain3->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));

	mountain4 = new Platform(mountainMesh, "Mountain4");
	mountain4->setPosition(glm::vec3(223.0f, 0.0f, -275.0f));
	mountain4->setScale(glm::vec3(300.0f, 270.0f, 300.0f));
	mountain4->setRotation(glm::vec3(0.0f, 80.0f, 0.0f));
	mountain4->setUseOBBCollision(true);

	mountain5 = new Platform(mountainMesh, "Mountain5");
	mountain5->setPosition(glm::vec3(-223.0f, 0.0f, -275.0f));
	mountain5->setScale(glm::vec3(300.0f, 350.0f, 300.0f));
	mountain5->setRotation(glm::vec3(0.0f,80.0f, 0.0f));
	mountain5->setUseOBBCollision(true);


	mountain6 = new Platform(mountainMesh, "Mountain6");
	mountain6->setPosition(glm::vec3(50.0f, 0.0f, -470.0f));
	mountain6->setScale(glm::vec3(200.0f, 400.0f, 200.0f));
	mountain6->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));


	// NEW: Create spike hazards
	Mesh spikeMesh = loader.loadObj("Resources/Models/spikes.obj", texturesSpike); // Assuming spike uses rock texture
	spike1 = new Platform(spikeMesh, "Spike1");
	spike1->setPosition(glm::vec3(-100.0f, 1.0f, -200.0f));
	spike1->setScale(glm::vec3(50.0f, 5.0f, 25.0f));
	spike1->setIsHazard(true);
	
	
	spike2 = new Platform(spikeMesh, "Spike2");
	spike2->setPosition(glm::vec3(37.0f, 42.0f, -115.0f));
	spike2->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
	spike2->setIsHazard(true);
	
	spike3 = new Platform(spikeMesh, "Spike3");
	spike3->setPosition(glm::vec3(9.0f, 49.0f, -196.0f));
	spike3->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
	spike3->setIsHazard(true);

	
	


	// Register all platforms with the collision manager
	collisionManager.addCollidable(g_platform);
	collisionManager.addCollidable(platform1);
	collisionManager.addCollidable(platform2);
	collisionManager.addCollidable(platform3);
	collisionManager.addCollidable(platform4);
	collisionManager.addCollidable(platform5);
	collisionManager.addCollidable(platform6);

	collisionManager.addCollidable(fence);

	collisionManager.addCollidable(mountain1);
	collisionManager.addCollidable(mountain2);
	collisionManager.addCollidable(mountain3);
	collisionManager.addCollidable(mountain4);
	collisionManager.addCollidable(mountain5);
	collisionManager.addCollidable(mountain6);
	
	// NEW: Register spikes as hazards
	collisionManager.addCollidable(spike1);
	collisionManager.addCollidable(spike2);
    collisionManager.addCollidable(spike3);

	// Optional: set collision margin and enable debug output
	collisionManager.setCollisionMargin(0);
	// Disable verbose debug output in release runs to avoid console flooding and input lag
	collisionManager.setDebugOutput(false);  // Changed from true to false to reduce mouse-look lag

	//check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();

		//test mouse input
		if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			std::cout << "Pressing mouse button" << std::endl;
		}
		//// Code for the light ////

		sunShader.use();
		//camera fov
		glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(70.0f), window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		//Test for one Obj loading = light source

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		sun.draw(sunShader);

		//// End code for the light ////

		shader.use();

		///// Test Obj files for box ////

		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

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

		if (fence) fence->draw(shader, ViewMatrix, ProjectionMatrix);

		if (mountain1) mountain1->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain2) mountain2->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain3) mountain3->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain5) mountain4->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain4) mountain5->draw(shader, ViewMatrix, ProjectionMatrix);
		if (mountain6) mountain6->draw(shader, ViewMatrix, ProjectionMatrix);
		
		// In the render loop, after drawing other platforms:
		if (spike1) spike1->draw(shader, ViewMatrix, ProjectionMatrix);
		if (spike2) spike2->draw(shader, ViewMatrix, ProjectionMatrix);
		if (spike3) spike3->draw(shader, ViewMatrix, ProjectionMatrix);

		window.update();
	}

	// Remove from collision manager before deleting
	collisionManager.clearAll();

	// cleanup
		// In cleanup section:
	delete spike1;
	spike1 = nullptr;
	delete spike2;
	spike2 = nullptr;
	delete spike3;
	spike3 = nullptr;


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
	delete fence;
	fence = nullptr;

	// NEW: Cleanup spike hazards
	delete spike1;
	spike1 = nullptr;
	delete spike2;
	spike2 = nullptr;
	delete spike3;
	spike3 = nullptr;
}

//basically game loop input processing
void processKeyboardInput()
{
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

	// debug : reset player position with P key
	static bool pWasPressed = false;
	bool pPressed = window.isPressed(GLFW_KEY_P);
	if (pPressed && !pWasPressed)
	{
		resetPlayer();
		std::cout << "\n[RESET] Player position reset to spawn!" << std::endl;
	}
	pWasPressed = pPressed;

	// Get current position
	glm::vec3 currentPos = camera.getCameraPosition();

	// calculate movement direction based on input
	glm::vec3 moveDirection(0.0f);

	// get forward and right vectors from camera (ignore y component for movement)
	glm::vec3 forward = camera.getCameraViewDirection();
	forward.y = 0.0f;
	if (glm::length(forward) > 0.0001f) forward = glm::normalize(forward);
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
	bool spacePressed = window.isPressed(GLFW_KEY_SPACE);
	if (spacePressed && !spaceWasPressed && playerPhysics.isGrounded)
	{
		playerPhysics.jump();
	}
	spaceWasPressed = spacePressed;

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

		// Check for hazard collision
		Platform* hitPlatform = dynamic_cast<Platform*>(info.collidable);
		if (hitPlatform && hitPlatform->getIsHazard())
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

		// For ANY collision (including side walls), check if we're still on ground
		// This is crucial - wall collisions should NOT break ground contact!
		if (!groundContactThisFrame)
		{
			glm::vec3 probePos = proposed;
			probePos.y -= 0.3f;

			if (collisionManager.resolvePointAgainstAll(probePos, PLAYER_EYE_HEIGHT))
			{
				const auto& probeInfo = collisionManager.getLastCollisionInfo();
				if (probeInfo.face == CollisionManager::ContactFace::Top)
				{
					groundContactThisFrame = true;
					playerPhysics.isGrounded = true;
					// Also reset vertical velocity when confirmed on ground
					if (playerPhysics.velocity.y < 0.0f)
					{
						playerPhysics.velocity.y = 0.0f;
					}
				}
			}
		}
	}
	else
	{
		// No collision - check if we're still on ground (edge detection)
		if (playerPhysics.isGrounded && playerPhysics.velocity.y <= 0.1f)
		{
			glm::vec3 probePos = proposed;
			probePos.y -= 0.3f;

			if (collisionManager.resolvePointAgainstAll(probePos, PLAYER_EYE_HEIGHT))
			{
				const auto& probeInfo = collisionManager.getLastCollisionInfo();
				if (probeInfo.face == CollisionManager::ContactFace::Top)
				{
					groundContactThisFrame = true;
					playerPhysics.isGrounded = true;
				}
			}
		}
	}

	// Update grounded state
	if (!groundContactThisFrame)
	{
		playerPhysics.isGrounded = false;
	}

	// Apply the resolved position to the camera
	camera.setCameraPosition(proposed);

	// Print player status to console
	playerPhysics.printStatus(proposed);
}

void resetPlayer()
{
	camera.setCameraPosition(SPAWN_POSITION);
	playerPhysics.reset();
}


