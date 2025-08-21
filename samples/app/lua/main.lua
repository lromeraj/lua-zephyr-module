-- Simple Lua demo application

print("Welcome to the Lua demo application")
print("----------------------------------")

-- While loop: countdown from 5
count = 5
while count > 0 do
    print("Countdown: " .. count)
    count = count - 1
    z.sleep(1000)
end

print("Done")