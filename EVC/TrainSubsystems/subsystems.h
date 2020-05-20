#pragma once
void update_power_status();
void handle_brake_command();
void update_train_subsystems()
{
    update_power_status();
    handle_brake_command();
}