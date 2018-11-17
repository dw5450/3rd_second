#pragma once

#include "Renderer.h"
#include "object.h"

class ScnMgr {
private:
	//Object * m_TestObj;
	Object * m_Objects[MAX_OBJECTS];
	Renderer * m_Renderer;

	GLuint m_TestTexture = 0;
	GLuint m_seqTexture = 0;

public:
	ScnMgr();
	~ScnMgr();

	void RenderScene(void);

	void Update(float eTime);
	void DoCollisionTest();
	void ApplyForce(float x, float y, float eTime);

	void AddObject(float x, float y, float z,
				float sx, float sy,
				float vx, float vy);

	bool CollisionObj( Object * obj1, Object * obj2);

	int FindEmptyObjectSlot();
	void DeleteObject(int id);
	void GarbageCollector();


public:
	void Shoot(int ShootID);
	
};