/*jslint browser: true*/
/*global Audio, Drupal*/
/**
 * @file
 * Displays Audio viewer.
 */
(function (Drupal, once) {
  Drupal.behaviors.islandora_audio_captions = {
    attach: function (context, settings) {

      once('islandora_audio_captions', 'audio', context).forEach(function (element) {
        function parseTimestamp(timestamp) {
          const [hours, minutes, seconds] = timestamp.split(':').map(Number);
          return hours * 3600 + minutes * 60 + seconds;
        }
        function parseWebVTT(content) {
          timeMarkerRegex = new RegExp("^\\d{2}:\\d{2}:\\d{2}\\.\\d{3} --> \\d{2}:\\d{2}:\\d{2}\\.\\d{3}$");
          const lines = content.split('\n');
          const cues = [];
          let currentCue = null;

          for (const line of lines) {
            const trimmedLine = line.trim();
            if (trimmedLine === 'WEBVTT') {
              continue;
            }
            if (!trimmedLine) {
              // Empty line indicates the end of a cue
              if (currentCue) {
                cues.push(currentCue);
                currentCue = null;
              }
              continue;
            }

            if (timeMarkerRegex.test(trimmedLine)) {
              // Timing line
              currentCue = { start: '', end: '', text: '' };
              const [start, end] = trimmedLine.split(' --> ');
              currentCue.start = parseTimestamp(start);
              currentCue.end = parseTimestamp(end);
            } else if (currentCue) {
              // Text line
              currentCue.text += (currentCue.text ? '\n' : '') + trimmedLine;
            }
          }
          if (typeof currentCue !== 'undefined') {
            cues.push(currentCue);
          }
          return cues;
        }

        element.textTracks.addEventListener('change', function(e)  {
          // Which track is showing?
          element.setAttribute('data-showing', false);
          for (let i = 0; i < this.length; i++) {
            if (this[i].mode === 'showing') {
              // Build cues for this track, now we know the index.
              // @todo: Make this work with multiple audio elements.
              var src = document.querySelectorAll("track[kind='captions'], track[kind='subtitles']")[i].getAttribute('src');
              bindVttSource(src);
              element.setAttribute('data-showing', true);
            }
          }
        });

        // Invoke change event to get started.
        const changeEvent = new Event('change');
        element.textTracks.dispatchEvent(changeEvent);

        function bindVttSource(vtt_source) {

          fetch(vtt_source).then(response => {
            if (!response.ok) {
              throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.text();
          }).then(text => {
            const cues = parseWebVTT(text);

            // Create a Custom Event attached to the timeupdate event to pass cues along.
            element.addEventListener('timeupdate', (e) => {
              const captionEvent = new CustomEvent('updateCaptions', { detail: cues })
              e.target.dispatchEvent(captionEvent);
            });
            element.addEventListener('updateCaptions', (e) => {
              if (!e.detail) {
                return;
              }
              // Check if a track is showing.
              var showing = e.target.getAttribute('data-showing');
              // Update the caption box.
              let captionsBox = e.target.parentElement.querySelector('div.audioTrack');
              if (showing === 'true') {
                // Grab the caption for the current time.
                const currentTime = e.target.currentTime;
                let cue = e.detail.find(
                  (cue) =>
                    cue !== null &&
                    currentTime >= cue.start &&
                    currentTime <= cue.end
                );
                if (cue?.text) {
                  captionsBox.innerHTML = cue.text.replace(/\n/g, "<br>");
                }
                else {
                  captionsBox.innerHTML = '';
                }
              }
              else {
                captionsBox.innerHTML = '';
              }
            });
          });
        }
      })
    }
  }
})(Drupal, once);
