% =========================================================================
% Script: Plot PM2.5 vs Time with Peak Markers (Toolbox-Free)
% =========================================================================

clear; clc; close all;

% 1. Specify file name
filename = '60% motor power 2.CSV'; % Change to '80% motor power.CSV' if needed

% Verify file existence before running
if ~isfile(filename)
    error('File "%s" was not found in MATLAB''s current directory.', filename);
end

% 2. Read table data
opts = detectImportOptions(filename);
data = readtable(filename, opts);

% 3. Extract Time & PM2.5 Values
time = datetime(data.Timestamp, 'InputFormat', 'MM/dd/yyyy hh:mm:ss a');
pm25 = data.PM2_5; % MATLAB converts 'PM2.5' header to 'PM2_5'
elapsedTimeSec = seconds(time - time(1));

% 4. Native Peak Detection (No toolboxes required)
% Identifies points greater than or equal to adjacent neighbor points
isSpike = [false; (pm25(2:end-1) > pm25(1:end-2)) & (pm25(2:end-1) >= pm25(3:end)); false];
peakIdx = find(isSpike);

% Fallback: If data is flat or step-like, highlight absolute maximum points
if isempty(peakIdx)
    peakIdx = find(pm25 == max(pm25));
end

% -------------------------------------------------------------------------
% Plotting
% -------------------------------------------------------------------------
figure('Name', 'PM2.5 Analysis', 'Color', 'w', 'Position', [100, 100, 900, 500]);
hold on;

% Plot raw sensor line with markers
plot(elapsedTimeSec, pm25, '-o', ...
    'Color', [0.12, 0.47, 0.71], ...
    'LineWidth', 1.5, ...
    'MarkerSize', 4, ...
    'MarkerFaceColor', [0.12, 0.47, 0.71], ...
    'DisplayName', 'PM_{2.5} Concentration');

% Highlight peaks with inverted red triangles
if ~isempty(peakIdx)
    plot(elapsedTimeSec(peakIdx), pm25(peakIdx), 'rv', ...
        'MarkerSize', 8, ...
        'MarkerFaceColor', 'r', ...
        'DisplayName', 'Peak Spike');

    % Text labels directly above peak markers
    for i = 1:length(peakIdx)
        idx = peakIdx(i);
        text(elapsedTimeSec(idx), pm25(idx) + 0.15, sprintf('%.1f', pm25(idx)), ...
            'HorizontalAlignment', 'center', ...
            'FontSize', 9, ...
            'FontWeight', 'bold', ...
            'Color', [0.7, 0, 0]);
    end
end

hold off;

% -------------------------------------------------------------------------
% Styling and Formatting
% -------------------------------------------------------------------------
grid on; grid minor;

title('PM_{2.5} Concentration vs. Elapsed Time', 'FontSize', 12, 'FontWeight', 'bold');
xlabel('Elapsed Time (seconds)', 'FontSize', 11);
ylabel('PM_{2.5} Concentration (\mug/m^3)', 'FontSize', 11);

% Dynamic Y-axis limits so labels aren't cut off
yMin = max(0, min(pm25) - 0.5);
yMax = max(pm25) + 0.8;
ylim([yMin, yMax]);

legend('Location', 'northeast');
set(gca, 'FontSize', 11);