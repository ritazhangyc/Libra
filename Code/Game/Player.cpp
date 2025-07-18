#include "Game/Player.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/NamedStrings.hpp"

Player::Player(Map* mapInstance, Vec2 const& spawnPosition, float spawnDegrees)
	:Entity(mapInstance, spawnPosition, spawnDegrees)
{
	StartUp();
}

Player::~Player()
{

}

void Player::StartUp()
{
	m_angularVelocity = g_gameConfigBlackboard.GetValue("playerTurnRate", 1.0f);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("playerPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("playerCosmeticRadius", 1.0f);
	m_health = g_gameConfigBlackboard.GetValue("playerHealth", 1);

	m_entityType = EntityType::ENTITY_TYPE_GOOD_PLAYER;
	m_entityFaction = EntityFaction::FACTION_GOOD;
	m_doesPushEntities = true;
	m_isPushedByEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;

	m_speed = 0.0f;
	m_velocity = m_speed * m_faceDirection;
	m_faceDirection = Vec2(1.0f, 0.0f);
	m_shootInterval = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 1.0f);

	SpawnTurret();
}

void Player::SpawnTurret()
{
	m_turret = new Turret();
	m_turret->m_faceDirection = m_faceDirection;
	m_turret->m_worldOrientationDegree = m_orientationDegrees;
	m_turret->m_localOrientationDegree = 0.0f;
}


void Player::RunFrame(float deltaSecond)
{
	BeginFrame();
	Update(deltaSecond);
	EndFrame();
}

void Player::BeginFrame()
{
	m_speed = 0.0f;
	m_velocity = m_speed * m_faceDirection;
}

void Player::Update(float deltaSecond)
{
	if (m_isDead)
	{
		g_theGame->PauseResumeGame();
		return;

	}
	UpdateInputKeyboard(deltaSecond);
	UpdateInputController(deltaSecond);
}

void Player::UpdateInputKeyboard(float deltaSecond)
{
	Vec2 inputVectorTurret = Vec2(0.0f, 0.0f);

	if (g_theInput->IsKeyDown('I'))
	{
		inputVectorTurret.y += 1.0f;
	}
	if (g_theInput->IsKeyDown('J'))
	{
		inputVectorTurret.x -= 1.0f;
	}
	if (g_theInput->IsKeyDown('K'))
	{
		inputVectorTurret.y -= 1.0f;
	}
	if (g_theInput->IsKeyDown('L'))
	{
		inputVectorTurret.x += 1.0f;
	}

	float inputMagnitudeTurret = GetClampedZeroToOne(inputVectorTurret.GetLength());
	float inputDegreeTurret = inputVectorTurret.GetOrientationDegrees();

	if (inputMagnitudeTurret) 
	{
		UpdateTurretRotation(deltaSecond, inputDegreeTurret);
	}

	Vec2 inputVectorPlayer = Vec2(0.0f, 0.0f);

	if (g_theInput->IsKeyDown('W'))
	{
		inputVectorPlayer.y += 1.0f;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		inputVectorPlayer.x -= 1.0f;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		inputVectorPlayer.y -= 1.0f;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		inputVectorPlayer.x += 1.0f;
	}

	float inputMagnitude = GetClampedZeroToOne(inputVectorPlayer.GetLength());
	float inputDegree = inputVectorPlayer.GetOrientationDegrees();

	if (inputMagnitude)
	{
		UpdatePlayerRotation(deltaSecond, inputDegree);
		UpdatePlayerPosition(deltaSecond, inputMagnitude);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
// 		if (!g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
// 		{
//			m_shootInterval = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 1.0f);
			m_map->SpawnNewEntity(EntityType::_GOOD_BULLET, m_position + m_turret->m_faceDirection * m_physicsRadius * 0.9f, m_turret->m_worldOrientationDegree);
			g_theGame->PlaySound(0);
			return;
// 		}
//		ShootBullet(deltaSecond);
	}

}

void Player::UpdateInputController(float deltaSecond)
{
	XboxController ctrler = g_theInput->GetController(0);
	if (ctrler.IsConnected() == false)
	{
		return;
	}

	float leftStickMagnitude = ctrler.GetLeftStick().GetMagnitude();
	float rightStickMagnitude = ctrler.GetRightStick().GetMagnitude();
	float leftStickDegree = ctrler.GetLeftStick().GetOrientationDegrees();
	float rightStickDegree = ctrler.GetRightStick().GetOrientationDegrees();
	
	if (rightStickMagnitude) 
	{
		UpdateTurretRotation(deltaSecond, rightStickDegree);
	}

	if (leftStickMagnitude) 
	{
		UpdatePlayerRotation(deltaSecond, leftStickDegree);
		UpdatePlayerPosition(deltaSecond, leftStickMagnitude);
	}

	if (ctrler.GetRightTrigger() == 1.0f)
	{
		ShootBullet(deltaSecond);
	}

}

void Player::UpdatePlayerRotation(float deltaSecond, float target)
{
	m_targetDegree = target;

	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, target, g_gameConfigBlackboard.GetValue("playerTurnRate", 180.0f) * deltaSecond);
	
	m_turret->m_worldOrientationDegree = m_orientationDegrees + m_turret->m_localOrientationDegree;

	m_faceDirection = Vec2(1.0f,0.0f).GetRotatedDegrees(m_orientationDegrees);

	m_turret->m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_turret->m_worldOrientationDegree);
}

void Player::UpdatePlayerPosition(float deltaSecond, float proportion)
{
	proportion = GetClampedZeroToOne(proportion);
	m_speed = proportion * g_gameConfigBlackboard.GetValue("playerDriveSpeed", 1.0f);
	m_velocity = m_speed * m_faceDirection;
	m_position +=m_velocity *deltaSecond;
}

void Player::UpdateTurretRotation(float deltaSecond, float target)
{

	GetTurret()->m_worldOrientationDegree = GetTurnedTowardDegrees(GetTurret()->m_worldOrientationDegree, target, g_gameConfigBlackboard.GetValue("playerGunTurnRate", 1.0f) * deltaSecond);

	m_turret->m_localOrientationDegree = m_turret->m_worldOrientationDegree - m_orientationDegrees;

	m_turret->m_faceDirection = Vec2(1.0f, 0.0f).GetRotatedDegrees(m_turret->m_worldOrientationDegree);
	//
	m_turret->m_orientationGoal = target;
}

void Player::ShootBullet(float deltaSecond)
{
	if (m_shootInterval > 0.0f)
	{
		m_shootInterval -= deltaSecond;
	}
	else
	{
		m_shootInterval = g_gameConfigBlackboard.GetValue("playerShootCooldownSeconds", 1.0f);
		m_map->SpawnNewEntity(EntityType::_GOOD_BULLET, m_position + m_turret->m_faceDirection * m_physicsRadius * 0.9f, m_turret->m_worldOrientationDegree);
		g_theGame->PlaySound(0);
	}
}

void Player::EndFrame()
{
}


void Player::Render()
{
	if (m_isDead)
	{
		return;
	}
	RenderTank();
	RenderTurret();
	if (m_invincible)
	{
		g_theRenderer->BindTexture(nullptr);
		DebugDrawRing(m_position, m_cosmeticRadius, 0.05f, Rgba8::WHITE);
	}
}

void Player::RenderTank() const
{
	std::vector <Vertex_PCU> tankVerts;
	Texture* tankBaseTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankBase.png");
	AABB2 tankAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(tankVerts, tankAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)tankVerts.size(), tankVerts.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(tankBaseTexture);
	g_theRenderer->DrawVertexArray((int)tankVerts.size(), tankVerts.data());

}

void Player::RenderTurret() const
{
	std::vector <Vertex_PCU> tankTurretVerts;
	Texture* tankTurretTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/PlayerTankTop.png");
	AABB2 tankAABB = AABB2(Vec2(-m_cosmeticRadius, -m_cosmeticRadius), Vec2(m_cosmeticRadius, m_cosmeticRadius));

	AddVertsForAABB2D(tankTurretVerts, tankAABB, Rgba8::WHITE);
	TransformVertexArrayXY3D((int)tankTurretVerts.size(), tankTurretVerts.data(), 1.0f, m_turret->m_worldOrientationDegree, m_position);
	g_theRenderer->BindTexture(tankTurretTexture);
	g_theRenderer->DrawVertexArray((int)tankTurretVerts.size(), tankTurretVerts.data());

}

void Player::HitByBullet(Entity* bullet)
{
	GetDamage(bullet->m_damage);
	bullet->Die();
}

void Player::GetDamage(int damageValue)
{
	if (m_invincible)
	{
		return;
	}
	m_health -= damageValue;
	SoundID	playerHurtSound = g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	g_theAudio->StartSound(playerHurtSound);
	if (m_health <= 0)
	{
		Die();
	}
}

void Player::Die()
{
	m_isDead = true;
}


void Player::RespawnPlayer()
{
	m_isDead = false;
	m_position = m_position;
	StartUp();
}

void Player::InvincibleModeOrNot()
{
	m_invincible = !m_invincible;
}

IntVec2 Player::GetTilePosition() const
{
	int tileX = RoundDownToInt(m_position.x);
	int tileY = RoundDownToInt(m_position.y);

	return IntVec2(tileX, tileY);
}

Turret* const Player::GetTurret() const
{
	return m_turret;
}

float const Player::GetTargetDegree() const
{
	return m_targetDegree;
}
