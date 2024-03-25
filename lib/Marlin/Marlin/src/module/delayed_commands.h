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
#pragma once

#include "../inc/MarlinConfig.h"
#include <deque>

struct delayed_command_queue_entry {
  uint8_t idx;
  std::function<void()> callable;
};

class DelayedCommands {

  public:
    void delay_command(std::function<void()> callable);
    void set_ready_to_run_idx(uint8_t idx) {
      if (idx == 0 || idx == ready_to_run_idx) {
        return;
      }
      ready_to_run_idx = idx;
      idx_updated_tick = ticks_ms();
    }


    uint8_t get_idx_for_next_block() {
      return last_used_idx;
    }

    void process_ready_queue();
    void reset();

  private:
    uint8_t last_used_idx = 0;
    uint8_t ready_to_run_idx = 0;
    uint8_t previous_processed_entry_idx = 0;
    std::deque<delayed_command_queue_entry> delayed_command_queue;
    uint32_t idx_updated_tick;
    uint32_t last_tick = 0;
};

extern DelayedCommands delayedCommandManager;
