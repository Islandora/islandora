<?php

namespace Drupal\islandora\Plugin\Action;

use Drupal\islandora\EventGenerator\EmitEvent;

/**
 * Emits a File event.
 *
 * @Action(
 *   id = "emit_file_event",
 *   label = @Translation("Emit a file event to a queue/topic"),
 *   type = "file"
 * )
 */
class EmitFileEvent extends EmitEvent {}
