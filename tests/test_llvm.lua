local acc_path = nil
for _, path in ipairs({ "../acc", "../acc.exe", "../acc.out" }) do
	if os.exists(path) then
		acc_path = path
	end
end

if acc_path == nil then
	error("acc executable not found.")
end

local function compare_file(p1, p2)
	local f1 = io.open(p1, "r");
	local f2 = io.open(p2, "r");
	repeat
		local l1 = f1:read()
		local l2 = f2:read()
		if l1 ~= l2 then
			f1:close()
			f2:close()
			return false
		end
	until l1 == nil
	f1:close()
	f2:close()
	return true
end

for _, input in ipairs(os.files("tests/input*")) do
	if input:endswith(".ans") then
		goto continue
	end

	local ans = input .. ".ans"
	if not os.exists(ans) then
		cprint("${orange} [WARN] not found answer for case %s.", input)
		goto continue
	end
	os.run("acc llvm %s out.ll", input)
	os.run("clang out.ll -o test.exe")
	local output = os.iorun("test.exe")
	local output_path = os.tmpfile()
	io.writefile(output_path, output)
	if compare_file(output_path, ans) then
		cprint("${green} [INFO] case %s: OK.", input)
	else
		cprint("${red} [INFO] case %s: FAILED.", input)
	end
	::continue::
end
