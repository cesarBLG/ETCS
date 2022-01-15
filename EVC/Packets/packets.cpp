/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "packets.h"
#include "3.h"
#include "5.h"
#include "12.h"
#include "16.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "42.h"
#include "47.h"
#include "65.h"
#include "66.h"
#include "67.h"
#include "68.h"
#include "69.h"
#include "72.h"
#include "76.h"
#include "79.h"
#include "80.h"
#include "88.h"
#include "132.h"
#include "136.h"
#include "137.h"
ETCS_packet *ETCS_packet::construct(bit_manipulator &r)
{
    int pos = r.position;
    NID_PACKET_t NID_PACKET;
    r.peek(&NID_PACKET);
    ETCS_packet *p;
    switch ((unsigned char)NID_PACKET) {
        case 3: p = new NationalValues(); break;
        case 5: p = new Linking(); break;
        case 12: p = new Level1_MA(); break;
        case 16: p = new RepositioningInformation(); break;
        case 21: p = new GradientProfile(); break;
        case 27: p = new InternationalSSP(); break;
        case 41: p = new LevelTransitionOrder(); break;
        case 42: p = new SessionManagement(); break;
        case 47: p = new ConditionalLevelTransitionOrder(); break;
        case 65: p = new TemporarySpeedRestriction(); break;
        case 66: p = new TemporarySpeedRestrictionRevocation(); break;
        case 67: p = new TrackConditionBigMetalMasses(); break;
        case 68: p = new TrackCondition(); break;
        case 69: p = new TrackConditionStationPlatforms(); break;
        case 72: p = new PlainTextMessage(); break;
        case 76: p = new FixedTextMessage(); break;
        case 79: p = new GeographicalPosition(); break;
        case 80: p = new ModeProfile(); break;
        case 88: p = new LevelCrossingInformation(); break;
        case 132: p = new DangerForShunting(); break;
        case 136: p = new InfillLocationReference(); break;
        case 137: p = new StopIfInSR(); break;
        default: p = new ETCS_directional_packet(); break;
    }
    p->copy(r);
    if (r.position-pos != p->L_PACKET)
         r.error = true;
    return p;
}