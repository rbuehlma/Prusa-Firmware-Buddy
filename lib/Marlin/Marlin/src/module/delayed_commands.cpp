/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
 *
 */

#include "delayed_commands.h"
#include "stepper.h"
#include <thread>

DelayedCommands delayedCommandManager;

void DelayedCommands::delay_command(std::function<void()> callable) {
  delayed_command_queue_entry entry;
  last_used_idx++;
  if (last_used_idx == 0) {
    last_used_idx++;
  }
  entry.idx = last_used_idx;
  entry.callable = callable;
  delayed_command_queue.push_back(entry);
}

void DelayedCommands::process_ready_queue() {
  uint32_t now = ticks_ms();
  if (now - last_tick >= 10) {
    char buf[50];
    sprintf(buf, "Tick took %lu msec\n", now - last_tick);
    serialprintPGM(buf);
  }
  last_tick = now;
  int count = 0;
  delayed_command_queue_entry lastEntry;
  while (!delayed_command_queue.empty()) {
    delayed_command_queue_entry entry = delayed_command_queue.front();
    int16_t rolling_diff = ready_to_run_idx - entry.idx;
    bool is_entry_due = (rolling_diff >= 0 && rolling_diff < 128) || (rolling_diff < 0 && rolling_diff < -128);

    if (!is_entry_due) {
      break;
    }
    previous_processed_entry_idx = entry.idx;
    entry.callable();
    count++;
    delayed_command_queue.pop_front();
  }
  uint32_t diff = ticks_ms() - now;
  if (diff > 3) {
        char buf[50];
        sprintf(buf, "I2C took %lu msec\n", diff);
        serialprintPGM(buf);
  }
  if (count > 0 && (idx_updated_tick < now - 4)) {
    char buf[50];
    sprintf(buf, "DelayedCommands: Late: %lu\n", now - idx_updated_tick);
    serialprintPGM(buf);
  }
  if (count > 0 && false) {
    char buf[100];
    sprintf(buf, "DelayedCommands: Idx: %u, Actual X: %0.2f, Y: %0.2f\n", ready_to_run_idx, (stepper.position(X_AXIS) - 1400.0)/100.0,  (stepper.position(Y_AXIS) + 400.0)/100.0);
    serialprintPGM(buf);
  }
}

void DelayedCommands::reset() {
  delayed_command_queue.clear();
  last_used_idx = 0;
  ready_to_run_idx = 0;

  idx_updated_tick = 0;
  serialprintPGM("DelayedCommands: Reset done");
}

