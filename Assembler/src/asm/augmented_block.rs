// Copyright (c) 2021, COSIC-KU Leuven, Kasteelpark Arenberg 10, bus 2452, B-3001 Leuven-Heverlee, Belgium.
// Copyright (c) 2021, Cosmian Tech SAS, 53-55 rue La Boétie, Paris, France.

use super::AugmentedBlock;
use super::AugmentedStatement;
use std::cmp;
use std::cmp::Ordering;

impl<'a> AugmentedBlock<'a> {
    pub fn max_round_depth(&self) -> i32 {
        let mut rnd_depth = 0;
        for stmt in self.stmts.iter() {
            rnd_depth = cmp::max(rnd_depth, stmt.round_depth);
        }
        rnd_depth
    }

    pub fn max_blck_number(&self) -> i32 {
        let mut blck = 0;
        for stmt in self.stmts.iter() {
            blck = cmp::max(blck, stmt.blck);
        }
        blck
    }

    /* Find the first and last instruction with given round depth and blck
     * Note, these may not be contiguous to begin with.
     */
    pub fn start_stop(&self, round_depth: i32, blck: i32) -> (usize, usize) {
        let mut new = 0;
        let mut start = 0;
        let mut stop = 0;
        for (cnt, stmt) in self.stmts.iter().enumerate() {
            if stmt.round_depth == round_depth && stmt.blck == blck {
                stop = cnt;
                if new == 0 {
                    new = 1;
                    start = cnt;
                }
            }
        }
        (start, stop)
    }

    /* Same but only worry about the block.
     * This will be contiguous
     */
    pub fn start_stop_blck(&self, blck: i32) -> (usize, usize) {
        let mut new = 0;
        let mut start = 0;
        let mut stop = 0;
        for (cnt, stmt) in self.stmts.iter().enumerate() {
            if stmt.blck == blck {
                stop = cnt;
                if new == 0 {
                    new = 1;
                    start = cnt;
                }
            }
        }
        (start, stop)
    }
}

impl<'a> PartialEq for AugmentedStatement<'a> {
    fn eq(&self, other: &Self) -> bool {
        self.blck == other.blck
            && self.round_depth == other.round_depth
            && self.instruction_depth == other.instruction_depth
    }
}

impl<'a> Eq for AugmentedStatement<'a> {}

impl<'a> Ord for AugmentedStatement<'a> {
    fn cmp(&self, other: &Self) -> Ordering {
        match () {
            _ if self.blck < other.blck => Ordering::Less,
            _ if self.blck > other.blck => Ordering::Greater,
            _ if self.round_depth < other.round_depth => Ordering::Less,
            _ if self.round_depth > other.round_depth => Ordering::Greater,
            _ if self.instruction_depth < other.instruction_depth => Ordering::Less,
            _ if self.instruction_depth > other.instruction_depth => Ordering::Greater,
            _ => Ordering::Equal,
        }
    }
}

impl<'a> PartialOrd for AugmentedStatement<'a> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}
