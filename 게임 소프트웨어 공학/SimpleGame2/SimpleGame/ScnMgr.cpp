#include "stdafx.h"
#include "ScnMgr.h"
#include "Global.h"

ScnMgr::ScnMgr()
{
	m_Renderer = NULL;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		m_Objects[i] = NULL;
	}


	//Init Renderer
	m_Renderer = new Renderer(500, 500);

	//Load Test Textrue
	m_TestTexture = m_Renderer->CreatePngTexture("./Textures/texture.png");
	m_seqTexture = m_Renderer->CreatePngTexture("./Textures/Sonic.png");

	//Init Test obj
	m_Objects[HERO_ID] = new Object();
	m_Objects[HERO_ID]->SetPosition(0.f, 0.f, 1.f);
	m_Objects[HERO_ID]->SetSize(1.f, 1.f);
	m_Objects[HERO_ID]->SetVelocity(0.f, 0.f);
	m_Objects[HERO_ID]->SetAcc(0.f, 0.f);
	m_Objects[HERO_ID]->SetCoefFrict(0.7f);
	m_Objects[HERO_ID]->SetMass(0.1f);
	m_Objects[HERO_ID]->SetColor(1.f, 1.f, 1.f, 1.f);

	AddObject(1, 0, 1, 0.1f, 0.1f, 0, 0);
}

ScnMgr::~ScnMgr()
{
	// nullptr이아닐경우 다 진입한다.
	// 이렇게 딜리트를 해주는 
	if (m_Renderer) {
		delete m_Renderer;
		m_Renderer = NULL;
	}

	if (m_Objects[HERO_ID]) {
		delete m_Objects[HERO_ID];
		m_Objects[HERO_ID] = NULL;
	}
}

void ScnMgr::RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	float x, y, z, sx, sy, r, g, b, a;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (m_Objects[i] != NULL) {
			m_Objects[i]->GetPosition(&x, &y, &z);
			m_Objects[i]->GetSize(&sx, &sy);
			m_Objects[i]->GetColor(&r, &g, &b, &a);

			// Renderer Test
			//m_Renderer->DrawTextureRectHeight(x*100, y*100, 0, sx*100,sy*100, r, g, b, a, m_TestTexture, z * 100
			m_Renderer->DrawTextureRectSeqXY(x * 100, y * 100, 0, sx * 100, sy * 100, r, g, b, a, m_seqTexture, 0, 0, 8, 1);
		}
	}
	
}

//float g_temp = 5.f;

void ScnMgr::Update(float eTime)
{

	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if(m_Objects[i] != NULL)
			m_Objects[i]->Update(eTime);
	}
}

void ScnMgr::DoCollisionTest()
{
	for (int i = 0; i < MAX_OBJECTS; ++i)
	{
		if (m_Objects[i] == NULL) continue;

		for (int j = 0; j < MAX_OBJECTS; ++j)
		{
			if (m_Objects[j] == NULL) continue;

			if (i == j)continue;
				
			if (CollisionObj(m_Objects[i], m_Objects[j]))
			{
				m_Objects[i]->SetColor(1.0f, 0.f, 0.f, 1.f);
				m_Objects[j]->SetColor(1.0f, 0.f, 0.f, 1.f);
				float r;

				m_Objects[i]->GetColor(&r, &r, &r, &r);

				std::cout << r;

			}
		}
	}
}

void ScnMgr::ApplyForce(float x, float y, float eTime)
{
	m_Objects[HERO_ID]->ApplyForce(x, y,eTime);
}

void ScnMgr::AddObject(float x, float y, float z, float sx, float sy, float vx, float vy)
{
	int id = FindEmptyObjectSlot();

	if (id < 0)
		return;
	m_Objects[id] = new Object();

	m_Objects[id]->SetPosition(x, y, z);
	m_Objects[id]->SetSize(sx, sy);
	m_Objects[id]->SetVelocity(vx, vy);
	m_Objects[id]->SetAcc(0.f, 0.f);
	m_Objects[id]->SetCoefFrict(0.7f);
	m_Objects[id]->SetMass(0.1f);
	m_Objects[id]->SetColor(1.f, 1.f, 1.f, 1.f);

}


//두 오브젝트가 충돌할 경우 트루 반환 aabb 모델
bool ScnMgr::CollisionObj( Object * obj1,  Object * obj2)
{
	float obj1_centerX, obj1_centerY, obj1_z;
	float obj1_sizeX, obj1_sizeY;
	float obj2_centerX, obj2_centerY, obj2_z;
	float obj2_sizeX, obj2_sizeY;

	obj1->GetPosition(&obj1_centerX, &obj1_centerY, &obj1_z);
	obj1->GetSize(&obj1_sizeX, &obj1_sizeY);
	obj2->GetPosition(&obj2_centerX, &obj2_centerY, &obj2_z);
	obj2->GetSize(&obj2_sizeX, &obj2_sizeY);

	float obj1_minX = obj1_centerX - obj1_sizeX / 2.f;
	float obj1_maxX = obj1_centerX + obj1_sizeX / 2.f;
	float obj2_minX = obj2_centerX - obj2_sizeX / 2.f;
	float obj2_maxX = obj2_centerX + obj2_sizeX / 2.f;

	float obj1_minY = obj1_centerY - obj1_sizeY / 2.f;
	float obj1_maxY = obj1_centerY + obj1_sizeY / 2.f;
	float obj2_minY = obj2_centerY - obj2_sizeY / 2.f;
	float obj2_maxY = obj2_centerY + obj2_sizeY / 2.f;


	if (obj2_maxX < obj1_minX)
		return false;

	if (obj1_maxX < obj2_minX)
		return false;

	if (obj2_maxY < obj1_minY)
		return false;

	if (obj1_maxY < obj2_minY)
		return false;

	return true;
}

int ScnMgr::FindEmptyObjectSlot()
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (m_Objects[i] == NULL)
			return i;
	}


	std::cout << "No more empty slot" << std::endl;
	return -1;
}

void ScnMgr::DeleteObject(int id)
{
	if (m_Objects[id] != NULL)
		delete m_Objects[id];

	m_Objects[id] = NULL;
}

void ScnMgr::GarbageCollector()
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (m_Objects[i] != NULL)
		{
			float x, y, z;
			m_Objects[i]->GetPosition(&x, &y, &z);
			if (x > 2.5f || x < -2.5 || y > 2.5f || y < -2.5f)
			{
				DeleteObject(i);
			}
		}
	}
}


void ScnMgr::Shoot(int ShootID)
{
	if (ShootID == SHOOT_NONE)
	{
		return;
	}

	float amount = SHOOTAMOUNT;
	float bvX, bvY;

	bvX = 0.f;
	bvY = 0.f;

	switch (ShootID)
	{
	case SHOOT_UP:
		bvX = 0.f;
		bvY = amount;
		break;

	case SHOOT_DOWN:
		bvX = 0.f;
		bvY = -amount;
		break;

	case SHOOT_LEFT:
		bvX = -amount;
		bvY = 0.f;
		break;

	case SHOOT_RIGHT:
		bvX = amount;
		bvY = 0.f;
		break;
	}

	float pX, pY, pZ;
	m_Objects[HERO_ID]->GetPosition(&pX, &pY, &pZ);

	/*float vX, vY;
	m_Objects[HERO_ID]->GetVelocity(&vX, &vY);*/


	AddObject(pX, pY, pZ, 0.3f, 0.3f, bvX, bvY);

}

