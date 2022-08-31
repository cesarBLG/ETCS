/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
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
#include "sb_feedback.h"
#include "targets.h"
extern target MRDT;
target prevMRDT;
double T_bs1_prev;
const double k1 = 2.0;
bool Q_feedback_active = false;
const double T_bs1_locked = 0;
const double T_bs2_locked = 2;
bool Q_displaylocked_TD = false;
bool Q_displaylocked_SBI = false;
bool Q_displaylocked_P = false;
bool Q_Tbslocked;
double T_bs_feedback;
extern MonitoringStatus monitoring;
double p0;
double p1;
double p2;
double p3;
void start_sb_feedback(double p0)
{
	::p0 = p0;
	double p1 = p0-30;
	double p2 = p0-60;
	double p3 = p0-150;
}
void sb_feedback()
{
    double p_cylinder;
    double p = 500-p_cylinder/k1;
	if (monitoring == TSM || monitoring == CSM) {
		if (Q_Tbslocked) {
			T_bs1 = T_bs1_locked;
			T_bs2 = T_bs2_locked;
		} else if (p > p2) {
			if (Q_feedback_active || p <= p1) {
				Q_feedback_active = true;
				T_bs_feedback = MRDT.T_bs * (p - p3) / (p0 - p3);
				T_bs1 = T_bs2 = T_bs_feedback;
				if (T_bs_feedback > MRDT.T_bs)
				    T_bs1 = T_bs2 = MRDT.T_bs;
				else
					T_bs2 = T_bs2_locked;
			} else {
				T_bs1 = MRDT.T_bs;
				T_bs2 = MRDT.T_bs;
			}
		} else {
			T_bs1 = T_bs1_locked;
			T_bs2 = T_bs2_locked;
			Q_feedback_active = true;
			Q_Tbslocked = true;
		}
	} else {
		T_bs1 = MRDT.T_bs;
		T_bs2 = MRDT.T_bs;
	}
	if (Q_feedback_active && T_bs1 < T_bs1_prev) {
		Q_displaylocked_P = true;
		Q_displaylocked_SBI = true;
		Q_displaylocked_TD = true;
	}
	T_bs1_prev = T_bs1; 
	//TODO
    /*if (Q_displaylocked_P && V_perm)
        Q_displaylocked_P = false;
	if (Q_displaylocked_SBI && V_sbi)
		Q_displaylocked_SBI = false;
	if (Q_displaylocked_TD && D_target < locked)
		Q_displaylocked_TD = false;*/

	if (!(MRDT == prevMRDT))
	{
		Q_displaylocked_P = false;
        Q_displaylocked_SBI = false;
        Q_displaylocked_TD = false;
    	prevMRDT = MRDT;
	}
	if (p > p0)
		p += 1.5;
	if (p0 > p && p > p0 - 30)
		p -= 1.5;
}