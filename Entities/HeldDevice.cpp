//////////////////////////////////////////////////////////////////////////////////////////
// File:            HeldDevice.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the HeldDevice class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "HeldDevice.h"
#include "Atom.h"
#include "RTEManagers.h"
#include "DDTTools.h"
#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "Actor.h"
#include "ACraft.h"
#include "AtomGroup.h"
#include "GUI/AllegroBitmap.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(HeldDevice, Attachable, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HeldDevice, effectively
//                  resetting the members of this abstraction level only.

void HeldDevice::Clear()
{
    m_HeldDeviceType = WEAPON;
    m_IsExplosiveWeapon = false;
    m_Activated = false;
    m_ActivationTmr.Reset();
    m_OneHanded = false;
	m_DualWieldable = false;
    m_StanceOffset.Reset();
    m_SharpStanceOffset.Reset();
    m_SharpAim = 0.0f;
    m_MaxSharpLength = 0;
    m_Supported = false;
    m_SupportOffset.Reset();
    m_BlinkTimer.Reset();
    m_PieSlices.clear();
    m_Loudness = -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int HeldDevice::Create()
{
    if (Attachable::Create() < 0)
        return -1;

    // Set MO Type.
    m_MOType = MovableObject::TypeHeldDevice;

    // Set HeldDeviceType based on tags
    if (IsInGroup("Weapons"))
        m_HeldDeviceType = WEAPON;
    else if (IsInGroup("Tools"))
        m_HeldDeviceType = TOOL;
    else if (IsInGroup("Shields"))
        m_HeldDeviceType = SHIELD;

    if (IsInGroup("Explosive Weapons"))
        m_IsExplosiveWeapon = true;
    else
        m_IsExplosiveWeapon = false;
    
    // Backwards compatibility so that the tag is added for sure
    if (m_HeldDeviceType == WEAPON)
        AddToGroup("Weapons");
    else if (m_HeldDeviceType == TOOL)
        AddToGroup("Tools");
    else if (m_HeldDeviceType == SHIELD)
        AddToGroup("Shields");
    
    // No Loudness set in the ini-file
    if (m_Loudness < 0)
    {
        if (m_HeldDeviceType == TOOL)
            m_Loudness = 0.5;   // Force tools to make less noise
        else
            m_Loudness = 1.0;
    }
    
    // All HeldDevice:s by default avoid hitting and getting physically hit by AtomGoups when they are at rest
    m_IgnoresAGHitsWhenSlowerThan = 1.0;
    
    // By default, held items should not be able to be squished and destroyed into the ground at all
    m_CanBeSquished = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a HeldDevice to be identical to another, by deep copy.

int HeldDevice::Create(const HeldDevice &reference)
{
    Attachable::Create(reference);

    // Set MO Type.
    m_MOType = MovableObject::TypeHeldDevice;

    m_HeldDeviceType = reference.m_HeldDeviceType;

    m_Activated = reference.m_Activated;
    m_ActivationTmr = reference.m_ActivationTmr;

    m_OneHanded = reference.m_OneHanded;
	m_DualWieldable = reference.m_DualWieldable;
    m_StanceOffset = reference.m_StanceOffset;
    m_SharpStanceOffset = reference.m_SharpStanceOffset;
    m_SupportOffset = reference.m_SupportOffset;

    m_SharpAim = reference.m_SharpAim;
    m_MaxSharpLength = reference.m_MaxSharpLength;
    m_Supported = reference.m_Supported;
    m_Loudness = reference.m_Loudness;

    for (list<PieMenuGUI::Slice>::const_iterator itr = reference.m_PieSlices.begin(); itr != reference.m_PieSlices.end(); ++itr)
        m_PieSlices.push_back(*itr);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int HeldDevice::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "HeldDeviceType")
        reader >> m_HeldDeviceType;
    else if (propName == "OneHanded")
        reader >> m_OneHanded;
	else if (propName == "DualWieldable")
		reader >> m_DualWieldable;
	else if (propName == "StanceOffset")
        reader >> m_StanceOffset;
    else if (propName == "SharpStanceOffset")
        reader >> m_SharpStanceOffset;
    else if (propName == "SupportOffset")
        reader >> m_SupportOffset;
    else if (propName == "SharpLength")
        reader >> m_MaxSharpLength;
    else if (propName == "Loudness")
        reader >> m_Loudness;
    else if (propName == "AddPieSlice")
    {
        PieMenuGUI::Slice newSlice;
        reader >> newSlice;
        m_PieSlices.push_back(newSlice);
		PieMenuGUI::AddAvailableSlice(newSlice);
    }
    else
        // See if the base class(es) can find a match instead
        return Attachable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this HeldDevice with a Writer for
//                  later recreation with Create(Reader &reader);

int HeldDevice::Save(Writer &writer) const
{
    Attachable::Save(writer);
/*
    writer.NewLine();
    writer << "// 0 = Offensive Weapon, 1 = Tool, 2 = Shield";
    writer.NewProperty("HeldDeviceType");
    writer << m_HeldDeviceType;
*/
    writer.NewProperty("OneHanded");
    writer << m_OneHanded;
    writer.NewProperty("StanceOffset");
    writer << m_StanceOffset;
    writer.NewProperty("SharpStanceOffset");
    writer << m_SharpStanceOffset;
    writer.NewProperty("SupportOffset");
    writer << m_SupportOffset;
    writer.NewProperty("SharpLength");
    writer << m_MaxSharpLength;
    writer.NewProperty("Loudness");
    writer << m_Loudness;
    for (list<PieMenuGUI::Slice>::const_iterator itr = m_PieSlices.begin(); itr != m_PieSlices.end(); ++itr)
    {
        writer.NewProperty("AddPieSlice");
        writer << *itr;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the HeldDevice object.

void HeldDevice::Destroy(bool notInherited)
{

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current stance parent offset.

Vector HeldDevice::GetStanceOffset() const
{
    if (m_SharpAim > 0)
        return m_SharpStanceOffset.GetXFlipped(m_HFlipped);
    else
        return m_StanceOffset.GetXFlipped(m_HFlipped);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSupportPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the support handhold that this HeldDevice
//                  offers.

Vector HeldDevice::GetSupportPos() const
{
/*
    Vector rotOff(m_SupportOffset.GetYFlipped(m_HFlipped));
    rotOff.RadRotate(m_HFlipped ? (PI + m_Rotation) : m_Rotation);
    return m_Pos + rotOff;
*/
    return m_Pos + RotateOffset(m_SupportOffset);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMagazinePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the magazine or other equivalent point of
//                  this.

Vector HeldDevice::GetMagazinePos() const
{
    return m_Pos;    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.

bool HeldDevice::AddPieMenuSlices(PieMenuGUI *pPieMenu)
{
    // Add the custom scripted options of this specific device
    for (list<PieMenuGUI::Slice>::iterator itr = m_PieSlices.begin(); itr != m_PieSlices.end(); ++itr)
        pPieMenu->AddSlice(*itr);

    return false;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool HeldDevice::CollideAtPoint(HitData &hd)
{
    return Attachable::CollideAtPoint(hd);
/* Obsolete
    if (hd.pBody[HITOR] && g_MovableMan.IsOfActor(hd.pBody[HITOR]->GetID())) {
        MovableObject *pMO = g_MovableMan.GetMOFromID(hd.pBody[HITOR]->GetRootID());
        if (pMO && pMO->IsActor())
            dynamic_cast<Actor *>(pMO)->SetItemInReach(this);
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates this HDFirearm. Analogous to 'pulling the trigger'.

void HeldDevice::Activate()
{
    if (!m_Activated)
    {
        m_ActivationTmr.Reset();
        // Register alarming event!
        if (m_Loudness > 0)
            g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, m_Loudness));
    }
    m_Activated = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this HDFirearm's features. Analogous to 'releasing
//                  the trigger'.

void HeldDevice::Deactivate()
{
    m_Activated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.

bool HeldDevice::OnMOHit(MovableObject *pOtherMO)
{
/* The ACraft now actively suck things in with cast rays instead
    // See if we hit any craft with open doors to get sucked into
    ACraft *pCraft = dynamic_cast<ACraft *>(pOtherMO);

    if (!IsSetToDelete() && pCraft && (pCraft->GetHatchState() == ACraft::OPEN || pCraft->GetHatchState() == ACraft::OPENING))
    {
        // Detach from whomever holds this
        Detach();
        // Add (copy) to the ship's inventory
        pCraft->AddInventoryItem(dynamic_cast<MovableObject *>(this->Clone()));
        // Delete the original from scene - this is safer than 'removing' or handing over ownership halfway through MovableMan's update
        this->SetToDelete();
        // Terminate; we got sucked into the craft; so communicate this out
        return true;
    }
*/
    // Don't terminate, continue on
    return false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this, using its physical representation.
// Arguments:       None.
// Return value:    None.

void HeldDevice::Travel()
{
    Attachable::Travel();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this HeldDevice. Supposed to be done every frame.

void HeldDevice::Update()
{
    Attachable::Update();

    // Remove loose items that have completely disappeared into the terrain, unless they're pinned
    if (!m_pParent && m_PinStrength <= 0 && m_RestTimer.IsPastSimMS(20000) && m_CanBeSquished && m_pAtomGroup->RatioInTerrain() > 0.9)
        GibThis();

    if (m_Activated)
        m_RestTimer.Reset();

    ////////////////////////////////////////
    // Animate the sprite, if applicable

    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENMOVING && m_Activated)
        {
            float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % m_SpriteAnimDuration;
            m_Frame = floorf((cycleTime / (float)m_SpriteAnimDuration) * (float)m_FrameCount);
        }
    }

    if (!m_pParent) {

    }
    else {
        /////////////////////////////////
        // Update and apply rotations and scale

        // Taken care of by holder/owner Arm.
//        m_Pos += m_ParentOffset;
// Don't apply state changes to BITMAP anywhere else than Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this HeldDevice's current graphical representation to a
//                  BITMAP of choice.

void HeldDevice::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
/*
    // Draw suporting hand if applicable.
    if (m_Supported) {
        Vector handPos(m_Pos.GetFloored() +
                       RotateOffset(m_SupportOffset) +
                       (m_Recoiled ? m_RecoilOffset : Vector()) -
                       targetPos);
        handPos.m_X -= m_pSupportHand->GetWidth() >> 1;
        handPos.m_Y -= m_pSupportHand->GetHeight() >> 1;
        if (!m_HFlipped)
            m_pSupportHand->DrawTrans(pTargetBitmap, handPos.m_X, handPos.m_Y);
        else
            m_pSupportHand->DrawTransHFlip(pTargetBitmap, handPos.m_X, handPos.m_Y);
    }
*/
/*
#ifdef _DEBUG
    if (mode == g_DrawColor && !onlyPhysical)
    {
        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false, 122);
        m_pDeepGroup->Draw(pTargetBitmap, targetPos, false, 13);
    }
#endif // _DEBUG
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void HeldDevice::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible)
        return;

    Attachable::DrawHUD(pTargetBitmap, targetPos, whichScreen);

    if (!m_pParent)
    {
        // Only draw if the team viewing this has seen the space where this is located
        int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
        if (viewingTeam != Activity::NOTEAM)
        {
            if (g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam))
                return;
        }

        Vector drawPos = m_Pos - targetPos;
        // Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
        if (!targetPos.IsZero())
        {
            // Spans vertical scene seam
            int sceneWidth = g_SceneMan.GetSceneWidth();
            if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth)
            {
                if ((targetPos.m_X < 0) && (m_Pos.m_X > (sceneWidth - pTargetBitmap->w)))
                    drawPos.m_X -= sceneWidth;
                else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_Pos.m_X < pTargetBitmap->w))
                    drawPos.m_X += sceneWidth;
            }
            // Spans horizontal scene seam
            int sceneHeight = g_SceneMan.GetSceneHeight();
            if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight)
            {
                if ((targetPos.m_Y < 0) && (m_Pos.m_Y > (sceneHeight - pTargetBitmap->h)))
                    drawPos.m_Y -= sceneHeight;
                else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_Pos.m_Y < pTargetBitmap->h))
                    drawPos.m_Y += sceneHeight;
            }
        }

        char str[64];
        str[0] = 0;
        GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
        GUIFont *pTextFont = g_FrameMan.GetSmallFont();
        if (pSymbolFont && pTextFont) {
            AllegroBitmap pBitmapInt(pTargetBitmap);
            if (m_BlinkTimer.GetElapsedSimTimeMS() < 300) {
                str[0] = -42;
                str[1] = 0;
            }
            else if (m_BlinkTimer.GetElapsedSimTimeMS() < 600) {
                str[0] = -41;
                str[1] = 0;
            }
            else if (m_BlinkTimer.GetElapsedSimTimeMS() < 900) {
                str[0] = -40;
                str[1] = 0;
            }
            else if (m_BlinkTimer.GetElapsedSimTimeMS() < 1200) {
                str[0] = 0;
            }
            else
                m_BlinkTimer.Reset();

            pSymbolFont->DrawAligned(&pBitmapInt, drawPos.m_X - 1, drawPos.m_Y - 20, str, GUIFont::Centre);
    //        sprintf(str, "%.0f", m_PresetName);
            sprintf(str, "%s", m_PresetName.c_str());
//            sprintf(str, "%s", m_sClass.GetName().c_str());
            pTextFont->DrawAligned(&pBitmapInt, drawPos.m_X + 0, drawPos.m_Y - 29, str, GUIFont::Centre);
        }
    }
}

} // namespace RTE