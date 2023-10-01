use nix::sys::wait::*;
use nix::unistd::{chdir, execvp, fork, ForkResult};
use std::ffi::CString;
use std::io;
use std::io::{stdout, Write};
use std::process;

enum BuiltinCommand {
    Cd,
    Help,
    Exit,
}

fn toysh_cd(args: Vec<&str>) {
    if args[1].is_empty() {
        eprintln!("args not found.\nUsage: cd [PATH]");
    } else {
        chdir(args[1]).expect("cd failed")
    }
}

fn toysh_help() {
    println!("Toysh is based on Stephen Brennan's lsh");
    println!("Type program names and arguments, and hit enter.");
}

fn toysh_exit() {
    process::exit(0);
}

fn toysh_launch(args: Vec<&str>) {
    // a) fork
    match unsafe { fork() }.expect("fork failed") {
        ForkResult::Parent { child } => {
            // b) waitpid
            match waitpid(child, None).expect("wait_pid failed") {
                WaitStatus::Exited(pid, status) => {
                    return;
                }
                WaitStatus::Signaled(pid, status, _) => {
                    println!("signal!: pid={:?}, status={:?}", pid, status)
                }
                _ => println!("abnormal exit!"),
            }
        }
        ForkResult::Child => {
            // 引数の値を取得する。
            let cmd = CString::new(args[0].to_string()).unwrap();
            let arg = CString::new(args[1].to_string()).unwrap();

            // c) execvp
            execvp(&cmd, &[cmd.clone(), arg]).expect("execution failed.");
        }
    }
}

fn toysh_execute(args: Vec<&str>) {
    match args[0] {
        "cd" => toysh_cd(args),
        "help" => toysh_help(),
        "exit" => toysh_exit(),
        _ => toysh_launch(args),
    }
}

fn main() {
    // toysh_loop();
    let mut input_line = String::new();
    let mut real_args: Vec<&str>;
    let mut virtual_args: Vec<&str>;

    loop {
        print!("> ");
        stdout().flush().unwrap();
        io::stdin()
            .read_line(&mut input_line)
            .expect("input error.\nfailed read_line()");
        let line = input_line.clone();
        real_args = line.split(&[' ', '\t', '\r', '\n'][..]).collect();
        if !real_args[0].is_empty() {
            virtual_args = real_args.clone();
            toysh_execute(virtual_args);
        }
        input_line.clear();
        real_args.clear();
    }
}
