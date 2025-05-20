/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "packets.h"
#include "../Version/version.h"
#include "0.h"
#include "2.h"
#include "3.h"
#include "V1/3.h"
#include "5.h"
#include "6.h"
#include "12.h"
#include "16.h"
#include "21.h"
#include "27.h"
#include "V1/27.h"
#include "39.h"
#include "V1/39.h"
#include "40.h"
#include "41.h"
#include "42.h"
#include "45.h"
#include "46.h"
#include "49.h"
#include "51.h"
#include "52.h"
#include "57.h"
#include "58.h"
#include "63.h"
#include "64.h"
#include "65.h"
#include "66.h"
#include "67.h"
#include "68.h"
#include "69.h"
#include "70.h"
#include "71.h"
#include "72.h"
#include "V1/72.h"
#include "76.h"
#include "79.h"
#include "V1/79.h"
#include "80.h"
#include "V1/80.h"
#include "88.h"
#include "90.h"
#include "131.h"
#include "132.h"
#include "133.h"
#include "136.h"
#include "137.h"
#include "138.h"
#include "139.h"
#include "140.h"
#include "141.h"
#include "143.h"
#include "180.h"
#include "181.h"
#include "V1/200.h"
#include "V1/203.h"
#include "254.h"
ETCS_packet *ETCS_packet::construct(bit_manipulator &r, int m_version)
{
    int pos = r.position;
    NID_PACKET_t NID_PACKET;
    r.peek(&NID_PACKET);
    ETCS_packet *p = nullptr;
    switch ((unsigned char)NID_PACKET) {
        case 0: if (VERSION_X(m_version) > 1) p = new VirtualBaliseCoverMarker(); break;
        case 2: p = new SystemVersionOrder(); break;
        case 3:
            if (VERSION_X(m_version) == 1) p = new V1::NationalValues();
            else p = new NationalValues();
            break;
        case 5: p = new Linking(); break;
        case 6: p = new VirtualBaliseCoverOrder(); break;
        case 12: p = new Level1_MA(); break;
        case 16: p = new RepositioningInformation(); break;
        case 21: p = new GradientProfile(); break;
        case 27:
            if (VERSION_X(m_version) == 1) p = new V1::InternationalSSP();
            else p = new InternationalSSP();
            break;
        case 39:
            if (VERSION_X(m_version) == 1) p = new V1::TrackConditionChangeTractionSystem();
            else p = new TrackConditionChangeTractionSystem();
            break;
        case 40: if (VERSION_X(m_version) > 1) p = new TrackConditionChangeCurrentConsumption(); break;
        case 41: p = new LevelTransitionOrder(); break;
        case 42: p = new SessionManagement(); break;
        case 45: p = new RadioNetworkRegistration(); break;
        case 46: p = new ConditionalLevelTransitionOrder(); break;
        case 49: p = new ListSHBalises(); break;
        case 51: p = new AxleLoadSpeedProfile(); break;
        case 52: if (VERSION_X(m_version) > 1)p = new PermittedBrakingDistanceInformation(); break;
        case 57: p = new MovementAuthorityRequestParameters(); break;
        case 58: p = new PositionReportParameters(); break;
        case 63: p = new ListSRBalises(); break;
        case 64: if (VERSION_X(m_version) > 1) p = new InhibitionOfRevocableTSRL23(); break;
        case 65: p = new TemporarySpeedRestriction(); break;
        case 66: p = new TemporarySpeedRestrictionRevocation(); break;
        case 67: p = new TrackConditionBigMetalMasses(); break;
        case 68: p = new TrackCondition(); break;
        case 69: if (VERSION_X(m_version) > 1) p = new TrackConditionStationPlatforms(); break;
        case 70: p = new RouteSuitabilityData(); break;
        case 71: p = new AdhesionFactor(); break;
        case 72:
            if (VERSION_X(m_version) == 1) p = new V1::PlainTextMessage();
            else p = new PlainTextMessage();
            break;
        case 76: if (VERSION_X(m_version) > 1) p = new FixedTextMessage(); break;
        case 79:
            if (VERSION_X(m_version) == 1) p = new V1::GeographicalPosition();
            else p = new GeographicalPosition();
            break;
        case 80:
            if (VERSION_X(m_version) == 1) p = new V1::ModeProfile();
            else p = new ModeProfile();
            break;
        case 88: if (VERSION_X(m_version) > 1) p = new LevelCrossingInformation(); break;
        case 90: p = new TrackAheadFreeTransition(); break;
        case 131: p = new RBCTransitionOrder(); break;
        case 132: p = new DangerForShunting(); break;
        case 133: p = new RadioInfillAreaInformation(); break;
        case 136: p = new InfillLocationReference(); break;
        case 137: p = new StopIfInSR(); break;
        case 138: p = new ReversingAreaInformation(); break;
        case 139: p = new ReversingSupervisionInformation(); break;
        case 140: p = new TrainRunningNumberRBC(); break;
        case 141: p = new DefaultGradientTSR(); break;
        case 143: if (VERSION_X(m_version) > 1) p = new SessionManagementNeighbourRIU(); break;
        case 180: if (VERSION_X(m_version) > 1) p = new LSSMAToggleOrder(); break;
        case 181: if (VERSION_X(m_version) > 1) p = new GenericLSFunctionMarker(); break;
        case 200: if (VERSION_X(m_version) == 1) p = new V1::VirtualBaliseCoverMarker(); break;
        case 203: if (VERSION_X(m_version) == 1) p = new V1::NationalValuesBraking(); break;
        case 206: if (VERSION_X(m_version) == 1) p = new TrackCondition(); break;
        case 239: if (VERSION_X(m_version) == 1) p = new TrackConditionChangeTractionSystem(); break;
        case 254: p = new DefaultBaliseInformation(); break;
        default: break;
    }
    if (p == nullptr) {
        bool highery = false;
        for (int v : supported_versions) {
            if (VERSION_X(m_version)==VERSION_X(v) && VERSION_Y(m_version)>VERSION_Y(v))
                highery = true;
        }
        if (!highery)
            r.sparefound = true;
        p = new ETCS_directional_packet();
    }
    p->copy(r);
    if (NID_PACKET != 0 && r.position-pos != p->L_PACKET)
         r.error = true;
    return p;
}