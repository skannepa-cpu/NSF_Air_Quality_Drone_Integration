% =========================================================================
% Script: Plot PM2.5 vs Time from "80% motor power.CSV"
% =========================================================================

clear; clc; close all;

% 1. Define filename and load data
filename = '80% motor power.CSV';
opts = detectImportOptions(filename);

% Read the CSV file into a table
data = readtable(filename, opts);

% 2. Parse timestamps and extract PM2.5 values
% Formats timestamp string into datetime format matching 'MM/dd/yyyy hh:mm:ss a'
time = datetime(data.Timestamp, 'InputFormat', 'MM/dd/yyyy hh:mm:ss a');
pm25 = data.PM2_5; % MATLAB converts 'PM2.5' header to 'PM2_5'

% Calculate elapsed time in seconds from the start
elapsedTimeSec = seconds(time - time(1));


% -------------------------------------------------------------------------
% Plot 1: PM2.5 vs Clock Time
% -------------------------------------------------------------------------
figure('Name', 'PM2.5 vs Absolute Time', 'Color', 'w');

plot(time, pm25, 'b-', 'LineWidth', 1.5);
grid on;
title('PM_{2.5} Concentration vs. Clock Time (80% Motor Power)');
xlabel('Time (HH:MM:SS)');
ylabel('PM_{2.5} Concentration (\mug/m^3)');
xtickformat('HH:mm:ss');
set(gca, 'FontSize', 11);


% -------------------------------------------------------------------------
% Plot 2: PM2.5 vs Elapsed Time (Seconds)
% -------------------------------------------------------------------------
figure('Name', 'PM2.5 vs Elapsed Time', 'Color', 'w');

plot(elapsedTimeSec, pm25, 'r-', 'LineWidth', 1.5);
grid on;
title('PM_{2.5} Concentration vs. Elapsed Time (80% Motor Power)');
xlabel('Elapsed Time (seconds)');
ylabel('PM_{2.5} Concentration (\mug/m^3)');
set(gca, 'FontSize', 11);