wrk.host = "localhost"
wrk.port = "8000"
wrk.method = "POST"
wrk.path = "/msghash_check_c"

txtmsgfile = io.open("../request_spectre_test_msg.txt", "r")
txtmsg = txtmsgfile:read("*all")
txtmsgfile:close()

wrk.body = txtmsg

local some_thread = {}

function init(args)
  assert(#args == 1)
  protection = args[1] -- lua is 1-indexed
  wrk.path = wrk.path .. "?protection=" .. protection
  wrk.path = wrk.path .. "&hash=2CF24DBA5FB0A30E26E83B2AC5B9E29E1B161E5C1FA7425E73043362938B9824"
end

function setup(thread)
  some_thread = thread
end

function done(summary, latency, requests)
  protection = some_thread:get("protection")
  avg_lat = latency.mean
  tail_lat = latency:percentile(99.0)
  throughput = summary.requests / summary.duration * 1000000  -- summary.duration is in microseconds
  file = io.open("results/msghash_check_c_" .. protection .. ".txt", "w")
  file:write(string.format("Machine readable:\n%g\n%g\n%g\n\n", avg_lat, tail_lat, throughput))
  if avg_lat < 1000 then
    file:write(string.format("Avg latency: %g us\n", avg_lat))
  elseif avg_lat < 1000000 then
    file:write(string.format("Avg latency: %g ms\n", avg_lat/1000))
  else
    file:write(string.format("Avg latency: %g s\n", avg_lat/1000000))
  end
  if tail_lat < 1000 then
    file:write(string.format("99%% latency: %g us\n", tail_lat))
  elseif tail_lat < 1000000 then
    file:write(string.format("99%% latency: %g ms\n", tail_lat/1000))
  else
    file:write(string.format("99%% latency: %g s\n", tail_lat/1000000))
  end
  file:write(string.format("Throughput: %g req/s\n", throughput))
  file:close()
end
