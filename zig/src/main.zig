const std = @import("std");

const MAX_ARGS = 10;

const stdout_file = std.io.getStdOut().writer();
var bw = std.io.bufferedWriter(stdout_file);
const stdout = bw.writer();
const stderr = std.io.getStdErr().writer();

fn toysh_cd(path: []const u8) !void {
    try std.os.chdir(path);
}

fn toysh_help() !void {
    try stdout.print("Toysh is based on Stephen Brennan's lsh\n", .{});
    try stdout.print("Type program names and arguments, and hit enter.\n", .{});
}

fn toysh_exit() void {
    std.os.exit(1);
}

fn toysh_launch(args_ptr: [*:null]const ?[*:0]const u8, envp: [1:null]?[*:0]u8) !void {
    const fork_pid = try std.os.fork();
    if (fork_pid == 0) {
        // child process
        const result = std.os.execvpeZ(args_ptr[0].?, args_ptr, &envp);
        try stdout.print("{}\n", .{result});
    } else {
        // parent process
        const wait_result = std.os.waitpid(fork_pid, 0);
        if (wait_result.status != 0) {
            try stdout.print("終了コード: {}\n", .{wait_result.status});
        }
    }
}

fn toysh_execute(args: *std.mem.SplitIterator(u8, std.mem.DelimiterType.any)) !void {
    const max = 10;
    const envp = [_:null]?[*:0]u8{null};
    // var args_ptr: [*:null]const ?[*:0]const u8 = undefined;
    var args_ptr: [max:null]?[*:0]u8 = undefined;
    var i: u8 = 0;

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    // defer _ = gpa.deinit();

    while (args.next()) |token| {
        var a = try std.mem.Allocator.dupeZ(allocator, u8, token);
        args_ptr[i] = @as(*align(1) const [*:0]u8, @ptrCast(&a)).*;
        i += 1;
    } else {
        args_ptr[i] = null;
    }
    // こんな風に書きたいけど書けなかった。
    // switch (args_ptr[0]) {
    //     "cd" => try toysh_cd(args),
    //     "help" => try toysh_help(),
    //     "exit" => try toysh_exit(),
    //     else => try toysh_launch(&args_ptr, envp),
    // }

    var tmp_args = std.mem.splitAny(u8, args.buffer, " \t\r\n");
    var cmd = tmp_args.next().?;
    if (std.mem.eql(u8, cmd, "cd")) {
        if (tmp_args.next()) |path| {
            try toysh_cd(path);
        } else {
            try stderr.print("less argument\nUsage cd [PATH]\n", .{});
        }
    } else if (std.mem.eql(u8, cmd, "help")) {
        try toysh_help();
    } else if (std.mem.eql(u8, cmd, "exit")) {
        toysh_exit();
    } else {
        try toysh_launch(&args_ptr, envp);
    }
}

pub fn main() !void {
    var input_line = std.io.getStdIn().reader();
    var buf: [1000]u8 = undefined;
    var args: std.mem.SplitIterator(u8, std.mem.DelimiterType.any) = undefined;

    while (true) {
        try stdout.print("> ", .{});
        try bw.flush();

        if (try input_line.readUntilDelimiterOrEof(&buf, '\n')) |line| {
            if (line.len == 0) {
                continue;
            }
            args = std.mem.splitAny(u8, line, " \t\r\n");
        }

        try toysh_execute(&args);
    }
}
