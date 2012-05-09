<?php



/**
 * @file
 * Tagging Form???
 */

/**
 * Show subject tags ???
 * @param type $pid
 * @return string 
 */
function _show_subject_tags($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/dublin_core');
  module_load_include('inc', 'fedora_repository', 'api/tagging');
  $obj = new Fedora_Item($pid);
  $tags = new TagSet($obj);
  if (!empty($tags->tags)) {
    $items = array();
    foreach ($tags->tags as $tag) {
      $items[] = array(
        'data' => $tag['name'],
        'title' => $tag['creator'],
      );
    }
    return theme('item_list', $items);
  }
}

/**
 * Fedora repository image tagging form ????
 * @global type $base_url
 * @param type $form_state
 * @param type $pid
 * @return type 
 */
function fedora_repository_image_tagging_form($form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/dublin_core');
  module_load_include('inc', 'fedora_repository', 'api/tagging');
  global $base_url;

  if (!empty($form_state['post']['pid'])) {
    $pid = $form_state['post']['pid'];
  }
  $obj = new Fedora_Item($pid);
  $form['tags-wrapper'] = array(
    '#tree' => FALSE,
    '#prefix' => '<div class="clear-block" id="ilives_tag_wrapper">',
    '#suffix' => '</div>',
  );
  $form['tags-wrapper']['tags'] = array(
    '#prefix' => '<div id="tags"><ul class="tagset">',
    '#suffix' => '</ul></div>',
  );

  // Add the current tags to the form.
  $tagset = new TagSet($obj);
  $tags = array();
  foreach ($tagset->tags as $tag) {
    $form_tag = $form['tags-wrapper']['tags'][$tag['name']] = array(
      '#prefix' => '<li>',
      '#suffix' => '</li>',
    );
    
    $tag_title_text = t('Added by @creator.', array(
      '@creator' => $tag['creator'],
    ));
    $tag_mnpl_search_path = "fedora/repository/mnpl_advanced_search/tag:{$tag['name']}";
    $form_tag['tag'] = array(
      '#value' => l($tag['name'], $tag_mnpl_search_path, array('attributes' => array(
        'title' => $tag_title_text
      ))),
    );
    if (user_access('modify fedora datastreams') || user_access('add fedora tags')) {
      // Delete button for each existing tag.
      $form_tag['delete'] = array(
        '#type' => 'imagebutton',
        '#image' => drupal_get_path('module', 'fedora_repository') . '/images/remove_icon.png',
        '#default_value' => $tag['name'],
        '#title' => t('Delete this tag'),
      );
    }
  }
  if (user_access('modify fedora datastreams') || user_access('add fedora tags')) {
    $form['tags-wrapper']['addtag'] = array(
      '#type' => 'textfield',
      '#title' => t('New Tag'),
      '#size' => 30,
      '#description' => t('Add a subject tag to this item.'),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Add Tag',
      '#name' => 'add_tag',
    );
  }
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => (!empty($form_state['pid']) ? $form_state['pid'] : $pid),
  );
  if (empty($form_state['pid'])) {
    $form_state['pid'] = $pid;
  }

  return $form;
}

/**
 * Hook image button process ???
 * @param type $form
 * @return string 
 */
function hook_imagebutton_process($form) {
  $form['op_x'] = array(
    '#name' => $form['#name'] . '_x',
    '#input' => TRUE,
    '#button_type' => 'submit',
    '#form_submitted' => TRUE,
  );
  return $form;
}

/**
 * Fedora repository image tagging from submit ???
 * @global type $user
 * @param type $form
 * @param type $form_state 
 */
function fedora_repository_image_tagging_form_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'api/tagging');
  global $user;

  $item = new Fedora_Item($form_state['values']['pid']);
  $tagset = new TagSet($item);
  $addtag = trim($_POST['addtag']);
  if (!empty($addtag)) {
    array_push($tagset->tags, array('name' => $form_state['values']['addtag'], 'creator' => $user->name));
  }
  elseif (!empty($form_state['values']['delete'])) {
    for ($i = 0; $i < count($tagset->tags); $i++) {
      if ($tagset->tags[$i]['name'] == $form_state['clicked_button']['#value']) {
        unset($tagset->tags[$i]);
      }
    }
  }
  $_SESSION['fedora_tagged_image']['pid'] = $form_state['values']['pid'];
  $tagset->save();
}

