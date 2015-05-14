#include "particle_generator.h"

CParticleGenerator::CParticleGenerator(CEditor *pEditor)
{
	m_pEditor = pEditor;

	m_Actived = false;
}
CParticleGenerator::~CParticleGenerator()
{ }

void CParticleGenerator::Tick(CUIRect ToolBox, CUIRect ToolBar, CUIRect View)
{

}
