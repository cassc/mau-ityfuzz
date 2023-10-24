use std::env;
use std::fs;

use clap::{AppSettings, Parser};
use clap_verbosity_flag::Verbosity;
use ethers::{
    core::types::{Address},
    providers::{Middleware, Provider, Http},
};
use crate::{
    consts::{ ADDRESS_REGEX, BYTECODE_REGEX },
    io::{ logging::*, file::* },
    ether::evm::{ opcodes::opcode }
};


#[derive(Debug, Clone, Parser)]
#[clap(about = "Disassemble EVM bytecode to Assembly",
       after_help = "For more information, read the wiki: https://jbecker.dev/r/heimdall-rs/wiki",
       global_setting = AppSettings::DeriveDisplayOrder, 
       override_usage = "heimdall disassemble <TARGET> [OPTIONS]")]
pub struct DisassemblerArgs {
    
    /// The target to disassemble, either a file, bytecode, contract address, or ENS name.
    #[clap(required=true)]
    pub target: String,

    /// Set the output verbosity level, 1 - 5.
    #[clap(flatten)]
    pub verbose: clap_verbosity_flag::Verbosity,
    
    /// The output directory to write the disassembled bytecode to
    #[clap(long="output", short, default_value = "", hide_default_value = true)]
    pub output: String,

    /// The RPC provider to use for fetching target bytecode.
    #[clap(long="rpc-url", short, default_value = "", hide_default_value = true)]
    pub rpc_url: String,

    /// When prompted, always select the default value.
    #[clap(long, short)]
    pub default: bool,

}


pub fn disassemble(contract_bytecode: String, output_dir: String) -> String {
    use std::time::Instant;
    let now = Instant::now();

    let (logger, _)= Logger::new("TRACE");

    let mut program_counter = 0;
    let mut output: String = String::new();

    // Iterate over the bytecode, disassembling each instruction.
    let byte_array = contract_bytecode.chars()
        .collect::<Vec<char>>()
        .chunks(2)
        .map(|c| c.iter().collect::<String>())
        .collect::<Vec<String>>();

    while program_counter < byte_array.len(){

        let operation = opcode(&byte_array[program_counter]);
        let mut pushed_bytes: String = String::new();

        if operation.name.contains("PUSH") {
            let byte_count_to_push: u8 = operation.name.replace("PUSH", "").parse().unwrap();
        
            pushed_bytes = match  byte_array.get(program_counter + 1..program_counter + 1 + byte_count_to_push as usize) {
                Some(bytes) => bytes.join(""),
                None => {
                    break
                }
            };
            program_counter += byte_count_to_push as usize;
        }
        

        output.push_str(format!("{} {} {}\n", program_counter, operation.name, pushed_bytes).as_str());
        program_counter += 1;
    }

    logger.success(&format!("disassembled {} bytes successfully.", program_counter).to_string());

    write_file(&String::from(format!("{}/bytecode.evm", &output_dir)), &contract_bytecode);
    let file_path = write_file(&String::from(format!("{}/disassembled.asm", &output_dir)), &output);
    logger.info(&format!("wrote disassembled bytecode to '{}' .", file_path).to_string());

    logger.debug(&format!("disassembly completed in {} ms.", now.elapsed().as_millis()).to_string());
    
    return output
}