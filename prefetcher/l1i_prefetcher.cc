#include "ooo_cpu.h"

void O3_CPU::l1i_prefetcher_initialize() 
{
  cout << "CPU " << cpu << " Entangling prefetcher" << endl;

  l1i_cpu_id = cpu;
  l1i_init_stats_table();
  l1i_last_basic_block = 0;
  l1i_consecutive_count = 0;
  l1i_basic_block_merge_diff = 0;

  l1i_init_hist_table();
  l1i_init_timing_tables();
  l1i_init_entangled_table();
}

void O3_CPU::l1i_prefetcher_branch_operate(uint64_t ip, uint8_t branch_type, uint64_t branch_target)
{

}

void O3_CPU::l1i_prefetcher_cache_operate(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit)
{

    l1i_cpu_id = cpu;
  uint64_t line_addr = v_addr >> LOG2_BLOCK_SIZE;

  if (!cache_hit) assert(!prefetch_hit);
  if (!cache_hit) assert(l1i_find_timing_cache_entry(line_addr) == L1I_WAY);
  if (cache_hit) assert(l1i_find_timing_cache_entry(line_addr) < L1I_WAY);

  l1i_stats_table[cpu][(line_addr & L1I_STATS_TABLE_MASK)].accesses++;
  if (!cache_hit) {
    l1i_stats_table[cpu][(line_addr & L1I_STATS_TABLE_MASK)].misses++;
    if (l1i_ongoing_request(line_addr)
	&& !l1i_is_accessed_timing_entry(line_addr)) {
      l1i_stats_table[cpu][(line_addr & L1I_STATS_TABLE_MASK)].late++;
    }
  }
  if (prefetch_hit) {
    l1i_stats_table[cpu][(line_addr & L1I_STATS_TABLE_MASK)].hits++;
  }

  bool consecutive = false;
  
  if (l1i_last_basic_block + l1i_consecutive_count == line_addr) { // Same
    return;
  } else if (l1i_last_basic_block + l1i_consecutive_count + 1 == line_addr) { // Consecutive
    l1i_consecutive_count++;
    consecutive = true;
  }
      
  // Queue basic block prefetches
  uint32_t bb_size = l1i_get_bbsize_entangled_table(line_addr);
  if (bb_size) l1i_stats_basic_blocks[bb_size]++;
  for (uint32_t i = 1; i <= bb_size; i++) {
    uint64_t pf_addr = v_addr + i * (1<<LOG2_BLOCK_SIZE);
    if (!l1i_ongoing_request(pf_addr >> LOG2_BLOCK_SIZE)) {
      if (prefetch_code_line(pf_addr)) {
	l1i_add_timing_entry(pf_addr >> LOG2_BLOCK_SIZE, 0, L1I_ENTANGLED_TABLE_WAYS);
      }
    }
  }
  
  // Queue entangled and basic block of entangled prefetches
  uint32_t num_entangled = 0;
  for (uint32_t k = 0; k < L1I_MAX_ENTANGLED_PER_LINE; k++) {
    uint32_t source_set = 0;
    uint32_t source_way = L1I_ENTANGLED_TABLE_WAYS;
    uint64_t entangled_line_addr = l1i_get_entangled_addr_entangled_table(line_addr, k, source_set, source_way);
    if (entangled_line_addr && (entangled_line_addr != line_addr)) {
      num_entangled++;
      uint32_t bb_size = l1i_get_bbsize_entangled_table(entangled_line_addr);
      if (bb_size) l1i_stats_basic_blocks_ent[bb_size]++;
      for (uint32_t i = 0; i <= bb_size; i++) {
	uint64_t pf_line_addr = entangled_line_addr + i;
	if (!l1i_ongoing_request(pf_line_addr)) {
	  if (prefetch_code_line(pf_line_addr << LOG2_BLOCK_SIZE)) {
	    l1i_add_timing_entry(pf_line_addr, source_set, (i == 0) ? source_way : L1I_ENTANGLED_TABLE_WAYS);
	  }
	}
      }
    }
  }
  if (num_entangled) l1i_stats_entangled[num_entangled]++; 

  if (!consecutive) { // New basic block found
    uint32_t max_bb_size = l1i_get_bbsize_entangled_table(l1i_last_basic_block);

    // Check for merging bb opportunities
    if (l1i_consecutive_count) { // single blocks no need to merge and are not inserted in the entangled table
      if (l1i_basic_block_merge_diff > 0) {
	l1i_add_bbsize_table(l1i_last_basic_block - l1i_basic_block_merge_diff, l1i_consecutive_count + l1i_basic_block_merge_diff);
	l1i_add_bb_size_hist_table(l1i_last_basic_block - l1i_basic_block_merge_diff, l1i_consecutive_count + l1i_basic_block_merge_diff);
      } else {
	l1i_add_bbsize_table(l1i_last_basic_block, max(max_bb_size, l1i_consecutive_count));
   	l1i_add_bb_size_hist_table(l1i_last_basic_block, max(max_bb_size, l1i_consecutive_count));
      }
    }
  }
  
  if (!consecutive) { // New basic block found
    l1i_consecutive_count = 0;
    l1i_last_basic_block = line_addr;
  }  

  if (!consecutive) {
    l1i_basic_block_merge_diff = l1i_find_bb_merge_hist_table(l1i_last_basic_block);
  }
  
  // Add the request in the history buffer
  uint32_t pos_hist = L1I_HIST_TABLE_ENTRIES; 
  if (!consecutive && l1i_basic_block_merge_diff == 0) {
    if ((l1i_find_hist_entry(line_addr) == L1I_HIST_TABLE_ENTRIES)) {
      pos_hist = l1i_add_hist_table(line_addr);
    } else {
      if (!cache_hit && !l1i_ongoing_accessed_request(line_addr)) {
    	pos_hist = l1i_add_hist_table(line_addr);      
      }
    }
  }

  // Add miss in the latency table
  if (!cache_hit && !l1i_ongoing_request(line_addr)) {
    l1i_add_timing_entry(line_addr, 0, L1I_ENTANGLED_TABLE_WAYS);
  }
  
  uint32_t source_set = 0;
  uint32_t source_way = L1I_ENTANGLED_TABLE_WAYS;
  l1i_access_timing_entry(line_addr, pos_hist, source_set, source_way);

  // Update confidence if late
  if (source_way < L1I_ENTANGLED_TABLE_WAYS) {
    l1i_update_confidence_entangled_table(source_set, source_way, line_addr, false);
  }


}

void O3_CPU::l1i_prefetcher_cycle_operate()
{

 if (!all_warmed_up && all_warmup_complete > NUM_CPUS) {

    l1i_init_stats_table();
    all_warmed_up = true;
  }


}

void O3_CPU::l1i_prefetcher_cache_fill(uint64_t v_addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_v_addr)
{

}

void O3_CPU::l1i_prefetcher_final_stats()
{

  cout << "CPU " << cpu << " L1I Entangling prefetcher final stats" << endl;
  l1i_print_stats_table();


}
