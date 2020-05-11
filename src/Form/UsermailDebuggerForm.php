<?php


namespace Drupal\mail_debugger\Form;


use Drupal\Core\Config\ConfigFactoryInterface;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\KeyValueStore\KeyValueFactoryInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Class UsermailDebuggerForm.
 *
 * @package Drupal\mail_debugger\Form
 */
class UsermailDebuggerForm extends FormBase {

  /**
   * @var \Drupal\Core\KeyValueStore\KeyValueStoreInterface
   */
  protected $defaultsStorage;

  /**
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $userStorage;

  /**
   * @var \Drupal\Core\Config\ImmutableConfig
   */
  protected $userMailConfig;

  /**
   * UsermailDebuggerForm constructor.
   *
   * @param \Drupal\Core\KeyValueStore\KeyValueFactoryInterface $storageFactory
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entityTypeManager
   * @param \Drupal\Core\Config\ConfigFactoryInterface $configFactory
   *
   * @throws \Drupal\Component\Plugin\Exception\InvalidPluginDefinitionException
   * @throws \Drupal\Component\Plugin\Exception\PluginNotFoundException
   */
  public function __construct(KeyValueFactoryInterface $storageFactory,
                              EntityTypeManagerInterface $entityTypeManager,
                              ConfigFactoryInterface $configFactory) {
    $this->defaultsStorage = $storageFactory->get(static::class);
    $this->userStorage = $entityTypeManager->getStorage('user');
    $this->userMailConfig = $configFactory->get('user.mail');
  }

  /**
   * @inheritdoc
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('keyvalue'),
      $container->get('entity_type.manager'),
      $container->get('config.factory')
    );
  }

  /**
   * @inheritdoc
   */
  public function getFormId() {
    return "usermail_debugger_form";
  }

  /**
   * @inheritdoc
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    return [
      'user' => [
        '#title' => $this->t("Send for"),
        '#type' => 'entity_autocomplete',
        '#target_type' => 'user',
        '#default_value' => $this->userStorage->load($this->defaultsStorage->get('user')),
        '#selection_handler' => 'default:user',
        '#selection_settings' => [
          'include_anonymous' => FALSE,
        ],
        '#required' => TRUE,
      ],
      'operation' => [
        '#title' => $this->t("Subject"),
        '#type' => 'radios',
        '#options' => $this->getOperations(),
        '#default_value' => $this->defaultsStorage->get('operation'),
        '#required' => TRUE,
      ],
      'actions' => [
        '#type' => 'actions',
        'submit' => [
          '#type' => 'submit',
          '#value' => $this->t("Send"),
        ],
      ],
    ];
  }

  protected function getOperations() {
    $result = [];
    foreach ($this->userMailConfig->get() as $maybeOperation => $config) {
      // Operation?
      if (!empty($config['subject'])) {
        // Tokens intentionally unprocessed.
        $result[$maybeOperation] = $config['subject'];
      }
    }
    return $result;
  }

  /**
   * @inheritdoc
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $this->defaultsStorage->set('user', $form_state->getValue('user'));
    $this->defaultsStorage->set('operation', $form_state->getValue('operation'));

    /** @var \Drupal\user\Entity\User $user */
    $user = $this->userStorage->load($form_state->getValue('user'));
    $result = _user_mail_notify(
      $form_state->getValue('operation'),
      $user
    );

    if ($result) {
      $this->messenger()->addStatus($this->t("Sent a message to :mail.", [
        ':mail' => $user->getEmail(),
      ]));
    }
  }
}
